# mac80211_hwsim

## 1. mac80211_hwsim

- mac80211_hwsim is a Linux Kernel module that can be used to simulate arbitrary number of IEEE 802.11 radios for mac80211. It can be used to **test** most of the `mac80211` functionality and user-space tools (e.g: hostapd and wpa_supplicant) in a way that matches very closely with the normal case of using real WLAN hardware.

- mac80211 framework just sees **`mac80211_hwsim` as another hardware drivers** so there is no previous setting of the `mac80211` framework in order to work with `mac80211_hwsim` in testing environments.

- The main goal for `mac80211_hwsim` is to make it easier for developers to test their code and work with new features to mac80211, hostapd, and wpa_supplicant. The simulated radios do not have the limitations of real hardware, so it is easy to generate an arbitrary test setup and always reproduce the same setup for future tests. In addition, since all radio operation is simulated, any channel can be used in tests regardless of regulator rules.

- The software works tracking the channel of each virtual radio and copying all transmitted frames to each one of the currently enabled virtual radios that have set the same channel for transmitting.

### 1.1. wmediumd

- **Wmediumd** is an application based on the C programming language and was developed by a United States company called Cozybit.

- **Wmediumd** was created to perform emulation of the wireless environment on emulated networks created on Linux OS.

### 1.2. Mininet-WiFI

- Mininet-WiFi is a fork of the Mininet SDN network emulator.

## 2. Setup

- This section introduces how we simply set up `mac80211_hwsim` in your host.

### 2.1. AP and STA

- 1. Let's create two radios for mac80211 stack:

    ```bash
    modprobe mac80211_hwsim radios=2 dyndbg=+p
    ```

  - `dyndbg` is DYNAMIC debug message. It needs to be supported by the kernel. Check the result by `ip link`:

    ```text
    5: wlan0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc mq state DOWN mode DEFAULT group default qlen 1000
        link/ether 02:00:00:00:00:00 brd ff:ff:ff:ff:ff:ff
    6: wlan1: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc mq state DOWN mode DEFAULT group default qlen 1000
        link/ether 02:00:00:00:01:00 brd ff:ff:ff:ff:ff:ff
    7: hwsim0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT group default qlen 1000
        link/ieee802.11/radiotap 12:00:00:00:00:00 brd ff:ff:ff:ff:ff:ff
    ```

  - `hwsim0` is used to monitor all frames coming through to `mac80211_hwsim`.
  - In this case `wlan0` and `wlan1` will be assigned to `AP` role and `STA` role separately.
    - Since `wlan0` and `wlan1` are running on the same kernel resource, it could be loopback when we are testing both connections. To avoid that, a Linux feature called **NameSpace** can help us to isolate the kernel resource perfectly.

- 2. In the **main terminal**, we create a namespace for **a client**:

    ```bash
    sudo ip netns add client
    ip netns list   # Check list namespace
    ```

- 3. Open **second terminal** and run the new process space we just created:

    ```bash
    sudo ip netns exec client bash
    ```

- 4. Back to the **main terminal**, we assign the network resource `wlan1` to **second terminal**:

    ```bash
    sudo iw phy phy1 set netns name /run/netns/client
    ```

- 5. In **main terminal**, we will create a bridge interface and enslave `eth0` with it (no need to enslave `wlan0` as it's done by `hostapd`):

    ```bash
    sudo brctl addbr br0
    sudo brctl addif br0 eth0
    sudo ifconfig br0 192.168.42.1
    ```

- 6. Now run `hostapd`:

    ```bash
    sudo ifconfig wlan0 up
    sudo hostapd -i wlan0 hostapd.conf # -B running in background -f hostapd.log
    ```

- 7. Switch to `second terminal` and run `wpa_supplicant` as STA role:

    ```bash
    sudo wpa_supplicant -c wpa_supplicant.conf -i wlan1 # -B running in background -f wpa_supplicant.log
    ```

  - After the connection succeed, config IP for our STA:

    ```bash
    ifconfig wlan1 192.168.42.10 netmask 255.255.255.0
    ping 192.168.42.1
    ```

- 8. (OPTIONAL) You can check both logs to see if any problems occur. Also, you can use `tcpdump` to capture all frames via `hwsim0`:

    ```bash
    sudo ifconfig hwsim0 up
    sudo tcpdump -i hwsim0 -v
    ```
