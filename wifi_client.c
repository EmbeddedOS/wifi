#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/nl80211.h>
#include <linux/genetlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#define RATES           \
    "\x01\x04\x02\x04\x0B\x16\x32\x08\x0C\x12\x18\x24\x30\x48\x60\x6C"

#define PROBE_REQ       \
    "\x40\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xCC\xCC\xCC\xCC\xCC\xCC"  \
    "\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00"

extern unsigned int if_nametoindex (const char *__ifname);

struct nl80211_state {
  struct nl_sock *nl_sock;
  struct nl_cache *nl_cache;
  struct genl_family *nl80211;
};

int get_interface_idx(int fd, char* iface) {
  struct ifreq ifr;
  memset( &ifr, 0, sizeof( ifr ) );
  strncpy( ifr.ifr_name, iface, sizeof( ifr.ifr_name ) - 1 );
  if( ioctl( fd, SIOCGIFINDEX, &ifr ) < 0 ) {
    fprintf(stderr, "ioctl(SIOCGIFINDEX) failed\n");
    return -1;
  }
  return ifr.ifr_ifindex;
}

int linux_nl80211_init(struct nl80211_state *state) {
  int err;

  state->nl_sock = nl_socket_alloc();

  if (!state->nl_sock) {
    fprintf(stderr, "Failed to allocate netlink socket.\n");
    return -ENOMEM;
  }

  if (genl_connect(state->nl_sock)) {
    fprintf(stderr, "Failed to connect to generic netlink.\n");
    err = -ENOLINK;
    goto out_handle_destroy;
  }

  if (genl_ctrl_alloc_cache(state->nl_sock, &state->nl_cache)) {
    fprintf(stderr, "Failed to allocate generic netlink cache.\n");
    err = -ENOMEM;
    goto out_handle_destroy;
  }

  state->nl80211 = genl_ctrl_search_by_name(state->nl_cache, "nl80211");
  if (!state->nl80211) {
    fprintf(stderr, "nl80211 not found.\n");
    err = -ENOENT;
    goto out_cache_free;
  }

  return 0;

out_cache_free:
  nl_cache_free(state->nl_cache);
out_handle_destroy:
  nl_socket_free(state->nl_sock);
  return err;
}

void nl80211_cleanup(struct nl80211_state *state) {
  genl_family_put(state->nl80211);
  nl_cache_free(state->nl_cache);
  nl_socket_free(state->nl_sock);
}

/* From iw util.c */
int ieee80211_channel_to_frequency(int chan, enum nl80211_band band) {
  /* see 802.11 17.3.8.3.2 and Annex J
   * there are overlapping channel numbers in 5GHz and 2GHz bands */
  if (chan <= 0)
    return 0; /* not supported */
  switch (band) {
  case NL80211_BAND_2GHZ:
    if (chan == 14)
      return 2484;
    else if (chan < 14)
      return 2407 + chan * 5;
    break;
  case NL80211_BAND_5GHZ:
    if (chan >= 182 && chan <= 196)
      return 4000 + chan * 5;
    else
      return 5000 + chan * 5;
    break;
  case NL80211_BAND_60GHZ:
    if (chan < 5)
      return 56160 + chan * 2160;
    break;
  default:
    ;
  }
  return 0; /* not supported */
}

int nl80211_set_channel(struct nl80211_state *state, char* ifname, int channel) {
  enum nl80211_band band = (channel <= 14) ? NL80211_BAND_2GHZ : NL80211_BAND_5GHZ;
  unsigned int freq = ieee80211_channel_to_frequency(channel, band);
  unsigned int htval = NL80211_CHAN_NO_HT;
  struct nl_msg *msg = nlmsg_alloc();
  if (!msg) {
    fprintf(stderr, "failed to allocate netlink message\n");
    return 2;
  }

  genlmsg_put(msg, 0, 0, genl_family_get_id(state->nl80211), 0, 0, NL80211_CMD_SET_WIPHY, 0);
  unsigned int ifidx = if_nametoindex(ifname);
  NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ifidx);
  NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
  NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_CHANNEL_TYPE, htval);

  nl_send_auto_complete(state->nl_sock, msg);
  nlmsg_free(msg);
  return 0;
nla_put_failure:
  return -ENOBUFS;
}

int main(int argc, char* argv[]) {
  struct nl80211_state state;
  unsigned char h80211[4096];
  int err = 0;
  int ret;

  if (argc < 3) {
    fprintf(stderr, "Usage: %s [iface] [channel]\n", argv[0]);
    return -1;
  }

//   ret = linux_nl80211_init(&state);
//   if (ret < 0)
//     return 1;

  int fd_out = socket( PF_PACKET, SOCK_RAW, htons( ETH_P_ALL ));
  if (fd_out < 0) {
    fprintf(stderr, "socket(PF_PACKET) failed.\n");
    err = 1;
    goto close_fd;
  }

  int ifidx = get_interface_idx(fd_out, argv[1]);
  if (ifidx < 0)
    return 1;

//   ret = nl80211_set_channel(&state, argv[1], atoi(argv[2]));
//   if (ret < 0)
//     return 1;

  struct sockaddr_ll sll;
  memset( &sll, 0, sizeof( sll ) );
  sll.sll_family   = AF_PACKET;
  sll.sll_ifindex  = ifidx;
  sll.sll_protocol = htons( ETH_P_ALL );

  if( bind( fd_out, (struct sockaddr *) &sll, sizeof( sll ) ) < 0 ) {
    fprintf(stderr, "bind(ETH_P_ALL) failed\n");
    err = 1;
    goto close_fd;
  }

  // Packet Injection format is composed of:
  //   [ radiotap header  ]
  //   [ ieee80211 header ]
  //   [ payload ]
  // https://www.kernel.org/doc/Documentation/networking/mac80211-injection.txt

  unsigned char outbuf[4096];
  unsigned char u8aRadiotap[] = {
    0x00, 0x00, // <-- radiotap version
    0x0c, 0x00, // <- radiotap header length
    0x04, 0x80, 0x00, 0x00, // <-- bitmap
    0x00, // <-- rate
    0x00, // <-- padding for natural alignment
    0x18, 0x00, // <-- TX flags
  };
  unsigned long nb_pkt_sent = 0;

  int len = 24;
  memcpy(h80211, PROBE_REQ, 24);
  h80211[24] = 0x00;      //ESSID Tag Number
  h80211[25] = 0x00;      //ESSID Tag Length
  len += 2;
  memcpy(h80211+len, RATES, 16);
  len += 16;

  if( (len > 24) && (h80211[1] & 0x04) == 0 && (h80211[22] & 0x0F) == 0) {
    h80211[22] = (nb_pkt_sent & 0x0000000F) << 4;
    h80211[23] = (nb_pkt_sent & 0x00000FF0) >> 4;
  }

  memcpy(outbuf, u8aRadiotap, sizeof (u8aRadiotap) );
  memcpy(outbuf + sizeof (u8aRadiotap), h80211, len);
  len += sizeof (u8aRadiotap);

  ret = write(fd_out, outbuf, len);
  if (ret < 0) {
    fprintf(stderr, "write failed: %s.\n", strerror(errno));
  }

close_fd:
  close(fd_out);
 // nl80211_cleanup(&state);
  return err;
}
