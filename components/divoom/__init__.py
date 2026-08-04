import logging

import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome import pins
from esphome.components.esp32 import add_idf_sdkconfig_option
from esphome.const import (
    CONF_BROKER,
    CONF_CLIENT_ID,
    CONF_ID,
    CONF_MQTT,
    CONF_PASSWORD,
    CONF_PAYLOAD,
    CONF_PORT,
    CONF_TOPIC,
    CONF_USERNAME,
    CONF_WILL_MESSAGE,
)
from esphome.core import CORE

_LOGGER = logging.getLogger(__name__)

CODEOWNERS = ["@d03n3rfr1tz3"]
DEPENDENCIES = ["esp32", "network"]
AUTO_LOAD = ["async_tcp", "mdns"]

# bluetooth classic and BLE do not coexist on the classic ESP32 in this setup
CONFLICTS_WITH = ["esp32_ble", "esp32_ble_tracker", "esp32_improv", "bluetooth_proxy"]

CONF_BLUETOOTH_NAME = "bluetooth_name"
CONF_BLUETOOTH_RETRY = "bluetooth_retry"
CONF_BLUETOOTH_FILTER = "bluetooth_filter"
CONF_TCP_PORT = "tcp_port"
CONF_TCP_MAX_CLIENTS = "tcp_max_clients"
CONF_MQTT_TOPIC = "mqtt_topic"
CONF_STATUS_LED = "status_led"

divoom_ns = cg.esphome_ns.namespace("divoom")
DivoomComponent = divoom_ns.class_("DivoomComponent", cg.Component)


def validate_topic_format(value):
    """mirrors isValidTopicFormat from validate.h"""
    value = cv.string_strict(value)
    if value.count("%s") != 1:
        raise cv.Invalid("the topic has to contain exactly one '%s' placeholder")
    if "%" in value.replace("%s", ""):
        raise cv.Invalid("the topic must not contain any other format specifier")
    return value


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(DivoomComponent),
        cv.Optional(CONF_BLUETOOTH_NAME): cv.string_strict,
        cv.Optional(CONF_BLUETOOTH_RETRY, default=3): cv.int_range(min=0, max=255),
        cv.Optional(CONF_BLUETOOTH_FILTER, default=True): cv.boolean,
        cv.Optional(CONF_TCP_PORT, default=7777): cv.port,
        cv.Optional(CONF_TCP_MAX_CLIENTS, default=3): cv.int_range(min=1, max=16),
        cv.Optional(CONF_MQTT_TOPIC, default="divoom/%s"): validate_topic_format,
        cv.Optional(CONF_STATUS_LED): pins.internal_gpio_output_pin_number,
    }
).extend(cv.COMPONENT_SCHEMA)


def _final_validate(config):
    full_config = fv.full_config.get()
    mqtt_config = full_config.get(CONF_MQTT)
    if mqtt_config is None:
        return config

    # the PlatformIO build registers its last will programmatically, ESPHome only
    # knows the one from its own mqtt block, so it has to match by hand
    expected_topic = config[CONF_MQTT_TOPIC] % "proxy"
    will = mqtt_config.get(CONF_WILL_MESSAGE) or {}
    if will.get(CONF_TOPIC) != expected_topic or will.get(CONF_PAYLOAD) != "offline":
        _LOGGER.warning(
            "mqtt: will_message should be topic '%s' with payload 'offline', "
            "otherwise the availability topic stays online after a crash",
            expected_topic,
        )

    return config


FINAL_VALIDATE_SCHEMA = _final_validate


async def to_code(config):
    # ESPHome enables CONFIG_ARDUINO_SELECTIVE_COMPILATION and opts out of every
    # Arduino library it does not use itself; add_library re-enables them
    cg.add_library("BluetoothSerial", None)
    cg.add_library("WiFi", None)

    # bluetooth classic with SPP, which ESPHome does not enable on its own
    add_idf_sdkconfig_option("CONFIG_BT_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_BT_BLUEDROID_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_BT_CLASSIC_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_BT_SPP_ENABLED", True)
    add_idf_sdkconfig_option("CONFIG_BT_A2DP_ENABLE", False)
    add_idf_sdkconfig_option("CONFIG_BT_BLE_ENABLED", False)
    add_idf_sdkconfig_option("CONFIG_BTDM_CTRL_MODE_BR_EDR_ONLY", True)
    add_idf_sdkconfig_option("CONFIG_BTDM_CTRL_MODE_BLE_ONLY", False)
    add_idf_sdkconfig_option("CONFIG_BTDM_CTRL_MODE_BTDM", False)

    cg.add_build_flag("-DDIVOOM_PLATFORM_ESPHOME")
    cg.add_build_flag(f"-DBLUETOOTH_RETRY={config[CONF_BLUETOOTH_RETRY]}")
    cg.add_build_flag(
        f"-DBLUETOOTH_FILTER={'true' if config[CONF_BLUETOOTH_FILTER] else 'false'}"
    )
    cg.add_build_flag(f"-DTCP_MAX={config[CONF_TCP_MAX_CLIENTS]}")
    if CONF_STATUS_LED in config:
        cg.add_build_flag(f"-DDIVOOM_STATUS_LED={config[CONF_STATUS_LED]}")

    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_BLUETOOTH_NAME in config:
        cg.add(var.set_bluetooth_name(config[CONF_BLUETOOTH_NAME]))
    cg.add(var.set_tcp_port(config[CONF_TCP_PORT]))
    cg.add(var.set_mqtt_topic(config[CONF_MQTT_TOPIC]))

    # the connection itself belongs to ESPHomes mqtt component; the shared core
    # only needs the same values to decide whether MQTT is active at all
    mqtt_config = CORE.config.get(CONF_MQTT)
    if mqtt_config is not None:
        cg.add(var.set_mqtt_host(mqtt_config[CONF_BROKER]))
        cg.add(var.set_mqtt_port(mqtt_config[CONF_PORT]))
        cg.add(var.set_mqtt_user(mqtt_config.get(CONF_USERNAME, "")))
        cg.add(var.set_mqtt_pass(mqtt_config.get(CONF_PASSWORD, "")))
        cg.add(var.set_mqtt_client(mqtt_config.get(CONF_CLIENT_ID, "")))
