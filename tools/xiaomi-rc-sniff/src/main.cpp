#include <Arduino.h>
// esphome-xiaomi-ble-wake: shows what a Xiaomi Bluetooth remote sends when its TV is in deep standby.
//
// The ESP32 takes the TV's Bluetooth address, so its chip also reports the advertisements the
// remote directs at the TV (a chip only reports a directed advertisement addressed to itself).
// It prints every advertisement from the remote's address, with its type and raw bytes.
//
// Board: any ESP32 with BLE, Arduino core 3.x. Library: NimBLE-Arduino 2.x.
// Keep the TV in deep standby while testing: switched on, the TV would answer the remote first.

#include <NimBLEDevice.h>
#include "esp_mac.h"

// Example addresses: put yours.
static const uint8_t TV_MAC[6] = {0x60, 0x1D, 0x9D, 0x19, 0x48, 0x28};  // the TV's Bluetooth address
static const char* REMOTE = "00:e0:4c:fb:bc:21";                        // the remote's (lowercase), or "" for any "MI RC"

static const char* advType(uint8_t t) {
  switch (t) {
    case 0: return "ADV_IND (undirected, connectable)";
    case 1: return "ADV_DIRECT_IND (directed)";
    case 2: return "ADV_SCAN_IND (scannable)";
    case 3: return "ADV_NONCONN_IND (not connectable)";
    case 4: return "SCAN_RSP";
    default: return "?";
  }
}

class Logger : public NimBLEScanCallbacks {
  void onResult(const NimBLEAdvertisedDevice* d) override {
    std::string a = d->getAddress().toString();
    if (REMOTE[0] ? a != REMOTE : d->getName() != "MI RC") return;
    Serial.printf("%8lu ms  %s  addrType=%d  %s  rssi=%d  name='%s'\n", millis(), a.c_str(),
                  d->getAddressType(), advType(d->getAdvType()), d->getRSSI(), d->getName().c_str());
    Serial.print("           raw: ");
    for (uint8_t b : d->getPayload()) Serial.printf("%02X", b);
    Serial.println();
  }
};

void setup() {
  Serial.begin(115200);
  delay(500);
  esp_err_t err = esp_iface_mac_addr_set(TV_MAC, ESP_MAC_BT);
  Serial.printf("set BT address: %s\n", err == ESP_OK ? "ok" : esp_err_to_name(err));

  NimBLEDevice::init("");
  Serial.printf("ESP32 Bluetooth address: %s (must read 60:1d:9d:19:48:28)\n",
                NimBLEDevice::getAddress().toString().c_str());

  NimBLEScan* scan = NimBLEDevice::getScan();
  scan->setScanCallbacks(new Logger(), true);  // every packet, duplicates included
  scan->setActiveScan(false);
  scan->setInterval(100);
  scan->setWindow(100);
  scan->setDuplicateFilter(false);
  scan->start(0, false);
  Serial.println("Listening for the remote… TV in deep standby, then press Power on the remote");
}

void loop() { delay(1000); }
