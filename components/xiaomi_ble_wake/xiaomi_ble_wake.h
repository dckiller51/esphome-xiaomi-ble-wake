#pragma once

#include <cstdint>
#include <string>
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
namespace xiaomi_ble_wake {

/**
 * Wakes a Xiaomi TV or box (MediaTek "WoBLE") from deep standby the way its Bluetooth remote does.
 * In standby the TV's Bluetooth chip only listens for its remote's address; on Power the remote
 * sends an undirected advertisement whose last field (type 0xFE) holds the end of the TV's
 * address. The ESP32 takes the remote's address at boot and sends the same advertisement on wake().
 *
 * The two addresses come from the YAML, or are set after flashing (set_remote_mac_text /
 * set_tv_mac_text, e.g. from text entities): those are kept on the ESP32 and win over the YAML.
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

  /** The addresses as "AA:BB:CC:DD:EE:FF", or "" when not set. */
  std::string remote_mac_str() const { return format_(this->remote_); }
  std::string tv_mac_str() const { return format_(this->tv_); }

  /**
   * Sets and keeps an address typed after flashing ("" goes back to the YAML's). A new remote
   * address needs a restart (the ESP32 takes it at boot): the ESP32 restarts by itself.
   * False when the text isn't an address.
   */
  bool set_remote_mac_text(const std::string &text);
  bool set_tv_mac_text(const std::string &text);

  bool is_configured() const { return !is_zero_(this->remote_) && !is_zero_(this->tv_); }

 protected:
  struct Stored {
    uint8_t remote[6];
    uint8_t tv[6];
  };

  static void to_bytes_(uint64_t mac, uint8_t *out) {
    for (int i = 0; i < 6; i++) out[i] = (mac >> (8 * (5 - i))) & 0xFF;
  }
  static bool is_zero_(const uint8_t *mac) {
    for (int i = 0; i < 6; i++)
      if (mac[i] != 0) return false;
    return true;
  }
  static std::string format_(const uint8_t *mac);
  static bool parse_(const std::string &text, uint8_t *out);
  bool set_text_(const std::string &text, uint8_t *current, const uint8_t *yaml, bool restart);
  void save_();

  uint8_t remote_[6]{};
  uint8_t tv_[6]{};
  uint8_t yaml_remote_[6]{};
  uint8_t yaml_tv_[6]{};
  uint32_t duration_{3000};
  bool connectable_{true};
  ESPPreferenceObject pref_;
};

}  // namespace xiaomi_ble_wake
}  // namespace esphome
