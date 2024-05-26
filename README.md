# WiFi

## What is it?

- Everything about wifi:
  - 1. Documentation.
  - 2. WiFi client with raw socket in C.
  - 3. Virtual 802.11 kernel module to emulate wireless (WiFi) network.

## Load The kernel module to emulate hw

### Using mac80211_hwsim module

- [mac80211_hwsim](https://hackmd.io/@akiranet/r1OC8CaNv)

- Load module:

```bash
sudo modprobe mac80211_hwsim
```

- Enable monitor mode:

```bash
sudo ifconfig wlan0 down
sudo iwconfig wlan0 mode Monitor
sudo ifconfig wlan0 up
```

- (Optional) monitor with tcpdump:

```bash
sudo tcpdump -i wlan0
```

- Start `hostapd` ([hostapd-guide](https://wiki.gentoo.org/wiki/Hostapd)):

```bash
hostapd -B -f hostapd.log -i wlan0 hostapd.conf
```

### Using our virtual mac80211 kernel module (TODO)

```bash
sudo insmod vwlan.ko
```
