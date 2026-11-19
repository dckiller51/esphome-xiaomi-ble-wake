# esphome-xiaomi-ble-wake

[![CI](https://github.com/dckiller51/esphome-xiaomi-ble-wake/actions/workflows/ci.yml/badge.svg)](https://github.com/dckiller51/esphome-xiaomi-ble-wake/actions/workflows/ci.yml)
[![Ko-fi](https://img.shields.io/badge/Ko--fi-Buy_me_a_coffee-F16061?style=flat-square&logo=ko-fi&logoColor=white)](https://ko-fi.com/dckiller)

Wakes a **Xiaomi TV or box from deep standby** with an ESP32, the way its own Bluetooth remote does. An [ESPHome](https://esphome.io) external component: a button in Home Assistant (or anything that can press one) turns the TV on, even when its Wi-Fi is off in standby and the network can't reach it.

Made for [Astrion](https://github.com/dckiller51/astrion-custom-dashboard), whose Android TV pairing can't wake these TVs over the network, but it works on its own.

> **Tested** on a Xiaomi Mi TV (MiTV-MSSP2, Android 9, MediaTek MT7668 Bluetooth) with its *MI RC* remote. Other Xiaomi TVs and boxes with the same chip should work: see [Will it work with mine?](#will-it-work-with-mine)

## How it works

In deep standby, the TV turns its Wi-Fi off but keeps its Bluetooth chip listening (MediaTek's *Wake on BLE*). The chip only listens for **its remote's address**. When you press Power, the remote sends a Bluetooth advertisement, from its own address, whose last field (type `0xFE`) holds the end of the **TV's** address; the chip sees it and wakes the TV.

The component gives the ESP32's Bluetooth **the remote's address** at boot, and on `wake()` sends that same advertisement for 3 seconds. The real remote keeps working: the ESP32 only speaks when asked.

Captured with this repository's [sniffer](#the-sniffer); the TV's own bug report shows the chip's wake list (`rc list is …`).

## Requirements

- An **ESP32** with Bluetooth LE: ESP32, ESP32-C3, ESP32-S3… (not an ESP8266).
- ESPHome 2025 or later, `esp-idf` framework recommended.
- Within Bluetooth range of the TV (a few metres; place it where you'd hold the remote).
- Your TV's **Bluetooth address** and its **remote's address** ([how to find them](#find-the-two-addresses)).

## Install

```yaml
external_components:
  - source: github://dckiller51/esphome-xiaomi-ble-wake
    components: [xiaomi_ble_wake]

esp32_ble:

xiaomi_ble_wake:
  id: tv_wake
  remote_mac: "00:E0:4C:FB:BC:21"   # the TV's remote
  tv_mac: "60:1D:9D:19:48:28"       # the TV itself

button:
  - platform: template
    name: "Xiaomi TV wake"
    on_press:
      - lambda: id(tv_wake).wake();
```

Full files: [examples/standalone.yaml](examples/standalone.yaml) (an ESP32 of its own), [examples/ir-extender-addon.yaml](examples/ir-extender-addon.yaml) (added to an ESP32 [Astrion IR extender](https://github.com/dckiller51/astrion-ir-extender): one box for infrared and the wake).

| Option | | |
| :--- | :--- | :--- |
| `remote_mac` | **required** | The remote's Bluetooth address. The ESP32 takes it |
| `tv_mac` | **required** | The TV's Bluetooth address |
| `duration` | `3s` | How long to advertise (the remote does about 3 s) |
| `connectable` | `true` | `false`: the TV can't try to connect to the ESP32 once awake (see below) |

At boot the log shows `Xiaomi BLE wake:` with both addresses; on a press, `Waking the TV: ESP_OK`.

**"Bring the remote closer" on the TV.** Once awake, the TV takes the ESP32 for its remote and tries to connect; the ESP32 has no pairing keys, so it fails, and the TV asks for the remote for a few seconds. Harmless. `connectable: false` may avoid it, if your TV still wakes that way: try it.

## Use it

- **Home Assistant**: the button `button.xiaomi_tv_wake`, in a script, an automation or a dashboard.
- **Astrion**: a hotkey, a tile or an Activity pressing that button (`button.press`), then turning the TV on as usual.

One ESP32 wakes one TV (it takes that TV's remote's address).

## Find the two addresses

**The TV's Bluetooth address** (`tv_mac`)
- On the TV: *Settings › Device preferences › About › Status › Bluetooth address*.
- Or with ADB: `adb shell settings get secure bluetooth_address`

**The remote's address** (`remote_mac`)
- With ADB, on a Xiaomi: `adb shell getprop persist.bluetooth.wakeuprcaddr` (e.g. `00:E0:4C:FB:BC:21;`: keep the address, without the `;`).
- Without ADB: *Settings › Remotes & accessories*, open the remote: its address is often shown.
- Or the [sniffer](#the-sniffer).

ADB over Wi-Fi: on the TV, *About › Build* pressed 7 times, then *Developer options › USB debugging*; on a computer, `adb connect <tv-ip>:5555` and accept the prompt on the TV.

A new remote paired to the TV replaces the old one's address: put the new one in `remote_mac`.

## Will it work with mine?

Look for MediaTek's wake in the TV's bug report (`adb bugreport report.zip`, then search the text inside):

- `woble` and `rc list is <address>` in the kernel log: same mechanism, it should work.
- Nothing of the kind: another chip or another way of waking; the [sniffer](#the-sniffer) shows what your remote sends, open an issue with it.

## The sniffer

[tools/xiaomi-rc-sniff](tools/xiaomi-rc-sniff): a PlatformIO program (Arduino core 3.x, NimBLE-Arduino) that prints what the remote sends. The ESP32 takes the **TV's** address, so its chip also reports what the remote directs at the TV.

1. Put your TV's address in `TV_MAC`, your remote's in `REMOTE` (or `""` to show any *MI RC*).
2. Upload, open the monitor (115200).
3. Turn the TV off, wait 5 minutes (deep standby), press Power near the ESP32.

Each line shows the advertisement's type and raw bytes. On the tested TV: `ADV_IND` from the remote, `02010503FF000106084D4920524303021218040D040500020A0004FE284819`, every 10–50 ms for about 3 s; `FE 28 48 19` is the end of the TV's address `…:19:48:28`.

## 🔗 Related projects

* [Astrion Custom Dashboard (Android app / APK)](https://github.com/dckiller51/astrion-custom-dashboard)
* [Astrion IR Extender (ESPHome firmware)](https://github.com/dckiller51/astrion-ir-extender)
* [Astrion IR Sniffer](https://github.com/dckiller51/astrion-ir-sniffer)

## ☕ Support

If you find **esphome-xiaomi-ble-wake** useful and want to support its development, you can buy me a coffee!

[![Ko-fi](https://img.shields.io/badge/Buy_me_a_coffee-Ko--fi-F16061?style=for-the-badge&logo=ko-fi&logoColor=white)](https://ko-fi.com/dckiller)

## Changes

See the [CHANGELOG](CHANGELOG.md).

## License

GPL-3.0, like Astrion.
