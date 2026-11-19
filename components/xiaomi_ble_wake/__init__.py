"""Wakes a Xiaomi TV or box from deep standby the way its Bluetooth remote does (esphome-xiaomi-ble-wake)."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_DURATION, CONF_ID

DEPENDENCIES = ["esp32_ble"]

CONF_REMOTE_MAC = "remote_mac"
CONF_TV_MAC = "tv_mac"
CONF_CONNECTABLE = "connectable"

xiaomi_ble_wake_ns = cg.esphome_ns.namespace("xiaomi_ble_wake")
XiaomiBleWake = xiaomi_ble_wake_ns.class_("XiaomiBleWake", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(XiaomiBleWake),
        cv.Optional(CONF_REMOTE_MAC): cv.mac_address,
        cv.Optional(CONF_TV_MAC): cv.mac_address,
        cv.Optional(CONF_DURATION, default="3s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_CONNECTABLE, default=True): cv.boolean,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # Optional: a ready-made firmware gets them after flashing (set_remote_mac_text / set_tv_mac_text).
    if CONF_REMOTE_MAC in config:
        cg.add(var.set_remote_mac(config[CONF_REMOTE_MAC].as_hex))
    if CONF_TV_MAC in config:
        cg.add(var.set_tv_mac(config[CONF_TV_MAC].as_hex))
    cg.add(var.set_duration(config[CONF_DURATION]))
    cg.add(var.set_connectable(config[CONF_CONNECTABLE]))
