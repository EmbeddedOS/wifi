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
sudo modprobe mac80211_hwsim radios=2 dyndbg=+p
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

- Start a access point with `hostapd` ([hostapd-guide](https://wiki.gentoo.org/wiki/Hostapd)):

```bash
sudo hostapd -i wlan0 hostapd.conf -dd
```

- Test connect access point with (wpa_supplicant):

```bash
sudo wpa_supplicant -c wpa_supplicant.conf -i wlan1
```

- Monitor all wifi network:

```bash
sudo ifconfig hwsim0 up
sudo tcpdump -i hwsim0 -v
```

### Using our virtual mac80211 kernel module (TODO)

```bash
sudo insmod vwlan.ko
```
