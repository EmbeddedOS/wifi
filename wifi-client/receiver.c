#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#define INTERFACE_MAX_LENGTH 20

void print_hx(const char *p, int length)
{
    if (length <= 0)
    {
        return;
    }

    printf("{");

    for (int i = 0; i < length; i++)
    {
        char c = p[i];
        printf("0x%2X", c & 0xff);
        if (i < length - 1)
        {
            printf(", ");
        }
    }

    printf("}\n");
}

void sigint_handler(int sig)
{
    printf("Exiting\n");
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    char interface[INTERFACE_MAX_LENGTH] = {0};
    int res = 0;
    int fd = 0;
    struct ifreq interface_idx = {0};
    struct sockaddr_ll socket_address = {0};
    socklen_t len = 0;

    if (argc < 2)
    {
        printf("Please specify interface name!\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        printf("Failed to disposition SIGINT!\n");
        exit(EXIT_FAILURE);
    }

    strncpy(interface, argv[1], INTERFACE_MAX_LENGTH);

    /* 1. Create raw socket layer 2. */
    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0)
    {
        printf("Cannot create socket: -%d\n", errno);
        exit(EXIT_FAILURE);
    }

    /* 2. Bind socket to the interface. */
    strncpy(interface_idx.ifr_name, interface, INTERFACE_MAX_LENGTH);
    if (ioctl(fd, SIOCGIFINDEX, &interface_idx) < 0)
    {
        printf("Cannot get interface index: -%d\n", errno);
        close(fd);
        exit(EXIT_FAILURE);
    }

    socket_address.sll_ifindex = interface_idx.ifr_ifindex;
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ALL);

    res = bind(fd, (struct sockaddr *)&socket_address, sizeof(socket_address));
    if (res < 0)
    {
        printf("Cannot bind to the interface: -%d\n", errno);
        close(fd);
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        char buffer[100] = {0};

        res = recvfrom(fd, buffer, sizeof(buffer), 0,
                       NULL,
                       NULL);
        if (res < 0)
        {
            printf("Cannot recvfrom: -%d\n", errno);
            close(fd);
            exit(EXIT_FAILURE);
        }

        print_hx(buffer, res);
    }

    return close(fd);
}