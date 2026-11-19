#include "xiaomi_ble_wake.h"
#include "esphome/core/log.h"

#include "esp_gap_ble_api.h"
#include "esp_mac.h"

namespace esphome {
namespace xiaomi_ble_wake {

static const char *const TAG = "xiaomi_ble_wake";

void XiaomiBleWake::setup() {
  esp_err_t err = esp_iface_mac_addr_set(this->remote_, ESP_MAC_BT);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Couldn't take the remote's Bluetooth address: %s", esp_err_to_name(err));
    this->mark_failed();
  }
}

void XiaomiBleWake::dump_config() {
  ESP_LOGCONFIG(TAG, "Xiaomi BLE wake:");
  ESP_LOGCONFIG(TAG, "  Remote: %02X:%02X:%02X:%02X:%02X:%02X", this->remote_[0], this->remote_[1],
                this->remote_[2], this->remote_[3], this->remote_[4], this->remote_[5]);
  ESP_LOGCONFIG(TAG, "  TV: %02X:%02X:%02X:%02X:%02X:%02X", this->tv_[0], this->tv_[1], this->tv_[2],
                this->tv_[3], this->tv_[4], this->tv_[5]);
  ESP_LOGCONFIG(TAG, "  Duration: %u ms, connectable: %s", (unsigned) this->duration_,
                this->connectable_ ? "yes" : "no");
}

void XiaomiBleWake::wake() {
  if (this->is_failed())
    return;
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
