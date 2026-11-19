#include "xiaomi_ble_wake.h"

#include <cctype>
#include <cstdio>
#include <cstring>

#include "esphome/core/application.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include "esp_gap_ble_api.h"
#include "esp_mac.h"

namespace esphome {
namespace xiaomi_ble_wake {

static const char *const TAG = "xiaomi_ble_wake";

void XiaomiBleWake::setup() {
  std::memcpy(this->yaml_remote_, this->remote_, 6);
  std::memcpy(this->yaml_tv_, this->tv_, 6);
  // Addresses set after flashing win over the YAML's.
  this->pref_ = global_preferences->make_preference<Stored>(fnv1_hash("xiaomi_ble_wake"));
  Stored s{};
  if (this->pref_.load(&s)) {
    if (!is_zero_(s.remote)) std::memcpy(this->remote_, s.remote, 6);
    if (!is_zero_(s.tv)) std::memcpy(this->tv_, s.tv, 6);
  }
  if (is_zero_(this->remote_)) {
    ESP_LOGW(TAG, "No remote address yet: set it, the ESP32 restarts to take it");
    return;
  }
  esp_err_t err = esp_iface_mac_addr_set(this->remote_, ESP_MAC_BT);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Couldn't take the remote's Bluetooth address: %s", esp_err_to_name(err));
    this->mark_failed();
  }
}

void XiaomiBleWake::dump_config() {
  std::string remote = this->remote_mac_str();
  std::string tv = this->tv_mac_str();
  ESP_LOGCONFIG(TAG, "Xiaomi BLE wake:");
  ESP_LOGCONFIG(TAG, "  Remote: %s", remote.empty() ? "(not set)" : remote.c_str());
  ESP_LOGCONFIG(TAG, "  TV: %s", tv.empty() ? "(not set)" : tv.c_str());
  ESP_LOGCONFIG(TAG, "  Duration: %u ms, connectable: %s", (unsigned) this->duration_,
                this->connectable_ ? "yes" : "no");
}

std::string XiaomiBleWake::format_(const uint8_t *mac) {
  if (is_zero_(mac)) return "";
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return buf;
}

bool XiaomiBleWake::parse_(const std::string &text, uint8_t *out) {
  // "00:E0:4C:FB:BC:21", "00-e0-…", "00E04CFBBC21": 12 hex digits, separators ignored.
  int n = 0;
  uint8_t bytes[6]{};
  for (char c : text) {
    if (c == ':' || c == '-' || c == ' ' || c == ';') continue;
    if (!std::isxdigit(static_cast<unsigned char>(c)) || n >= 12) return false;
    uint8_t v = std::isdigit(static_cast<unsigned char>(c)) ? c - '0' : (std::toupper(c) - 'A' + 10);
    bytes[n / 2] = (bytes[n / 2] << 4) | v;
    n++;
  }
  if (n != 12) return false;
  std::memcpy(out, bytes, 6);
  return true;
}

bool XiaomiBleWake::set_text_(const std::string &text, uint8_t *current, const uint8_t *yaml, bool restart) {
  uint8_t next[6];
  if (text.empty()) {
    std::memcpy(next, yaml, 6);
  } else if (!parse_(text, next)) {
    ESP_LOGW(TAG, "Not a Bluetooth address: '%s' (expected AA:BB:CC:DD:EE:FF)", text.c_str());
    return false;
  }
  if (std::memcmp(next, current, 6) == 0) return true;
  std::memcpy(current, next, 6);
  this->save_();
  std::string shown = format_(current);
  ESP_LOGI(TAG, "Address set: %s", shown.empty() ? "(none)" : shown.c_str());
  if (restart) {
    ESP_LOGI(TAG, "Restarting to take the remote's address");
    this->set_timeout("restart", 1000, []() { App.safe_reboot(); });
  }
  return true;
}

bool XiaomiBleWake::set_remote_mac_text(const std::string &text) {
  return this->set_text_(text, this->remote_, this->yaml_remote_, true);
}

bool XiaomiBleWake::set_tv_mac_text(const std::string &text) {
  return this->set_text_(text, this->tv_, this->yaml_tv_, false);
}

void XiaomiBleWake::save_() {
  Stored s{};
  // Only what differs from the YAML is kept, so a YAML change still applies.
  if (std::memcmp(this->remote_, this->yaml_remote_, 6) != 0) std::memcpy(s.remote, this->remote_, 6);
  if (std::memcmp(this->tv_, this->yaml_tv_, 6) != 0) std::memcpy(s.tv, this->tv_, 6);
  this->pref_.save(&s);
  global_preferences->sync();
}

void XiaomiBleWake::wake() {
  if (this->is_failed()) return;
  if (!this->is_configured()) {
    ESP_LOGW(TAG, "Set the remote's and the TV's addresses first");
    return;
  }
  // Captured from the remote (Xiaomi "MI RC"), TV in deep standby.
  uint8_t data[] = {
      0x02, 0x01, 0x05,                                       // flags: limited discoverable, LE only
      0x03, 0xFF, 0x00, 0x01,                                 // manufacturer data, company 0x0100
      0x06, 0x08, 'M', 'I', ' ', 'R', 'C',                    // shortened name
      0x03, 0x02, 0x12, 0x18,                                 // 16-bit service: HID
      0x04, 0x0D, 0x04, 0x05, 0x00,                           // class of device: remote control
      0x02, 0x0A, 0x00,                                       // TX power 0 dBm
      0x04, 0xFE, this->tv_[5], this->tv_[4], this->tv_[3]};  // Xiaomi wake: the TV's address end
  esp_err_t err = esp_ble_gap_config_adv_data_raw(data, sizeof(data));
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Advertising data refused: %s", esp_err_to_name(err));
    return;
  }
  // The data is set asynchronously: start a moment later.
  this->set_timeout("start", 100, [this]() {
    esp_ble_adv_params_t p = {};
    p.adv_type = this->connectable_ ? ADV_TYPE_IND : ADV_TYPE_NONCONN_IND;
    // 20 ms; a non-connectable one can't go below 100 ms on this Bluetooth version.
    p.adv_int_min = p.adv_int_max = this->connectable_ ? 0x20 : 0xA0;
    p.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    p.channel_map = ADV_CHNL_ALL;
    p.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
    esp_err_t e = esp_ble_gap_start_advertising(&p);
    ESP_LOGI(TAG, "Waking the TV: %s", esp_err_to_name(e));
    this->set_timeout("stop", this->duration_, []() {
      esp_ble_gap_stop_advertising();
      ESP_LOGD(TAG, "Advertising stopped");
    });
  });
}

}  // namespace xiaomi_ble_wake
}  // namespace esphome
