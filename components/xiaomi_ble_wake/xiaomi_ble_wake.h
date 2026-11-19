#pragma once

#include <cstdint>
#include "esphome/core/component.h"

namespace esphome {
namespace xiaomi_ble_wake {

/**
 * Wakes a Xiaomi TV or box (MediaTek "WoBLE") from deep standby the way its Bluetooth remote does.
 * In standby the TV's Bluetooth chip only listens for its remote's address; on Power the remote
 * sends an undirected advertisement whose last field (type 0xFE) holds the end of the TV's
 * address. The ESP32 takes the remote's address at boot and sends the same advertisement on wake().
 */
class XiaomiBleWake : public Component {
 public:
  void set_remote_mac(uint64_t mac) { to_bytes_(mac, this->remote_); }
  void set_tv_mac(uint64_t mac) { to_bytes_(mac, this->tv_); }
  void set_duration(uint32_t ms) { this->duration_ = ms; }
  void set_connectable(bool connectable) { this->connectable_ = connectable; }

  // Before esp32_ble starts the Bluetooth stack: its address must already be the remote's.
  float get_setup_priority() const override { return setup_priority::BUS; }
  void setup() override;
  void dump_config() override;

  /** Advertises like the remote for the configured duration. */
  void wake();

 protected:
  static void to_bytes_(uint64_t mac, uint8_t *out) {
    for (int i = 0; i < 6; i++) out[i] = (mac >> (8 * (5 - i))) & 0xFF;
  }

  uint8_t remote_[6]{};
  uint8_t tv_[6]{};
  uint32_t duration_{3000};
  bool connectable_{true};
};

}  // namespace xiaomi_ble_wake
}  // namespace esphome
