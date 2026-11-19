# Changelog

All notable changes to this project are documented here.
Format based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [0.1.0] - Unreleased

### Added

- **`xiaomi_ble_wake` ESPHome component**: wakes a Xiaomi TV or box from deep standby the way its Bluetooth remote does. The ESP32 takes the remote's address at boot and, on `wake()`, sends the remote's wake advertisement (its last field holds the end of the TV's address) for 3 seconds. Options: `remote_mac`, `tv_mac`, `duration`, `connectable`. Tested on a Mi TV MiTV-MSSP2 (Android 9, MediaTek MT7668) with its *MI RC* remote.
- **Sniffer** (`tools/xiaomi-rc-sniff`, PlatformIO): prints what a remote sends with its TV in deep standby. The ESP32 takes the TV's address, so its chip also reports what the remote directs at the TV.
- **Examples**: an ESP32 of its own, and the blocks to add to an ESP32 [Astrion IR extender](https://github.com/dckiller51/astrion-ir-extender).
- **README**: how it works, finding the TV's and the remote's addresses, checking whether a TV uses the same wake.
- **CI**: the component built on an ESP32, an ESP32-C3 and an ESP32-S3, and the sniffer built with PlatformIO.
