
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "divoom_component.h"

#include "config.h"
#include "settings.h"

#include "hardware_bluetoothctl.h"
#include "hardware_wifictl.h"

#include "input_base.h"
#include "output_base.h"

#include "esphome/core/application.h"
#include "esphome/core/log.h"

namespace esphome {
namespace divoom {

static const char *const TAG = "divoom";
static volatile bool failPending = false;
static const char *failReason = nullptr;

/**
 * setup functionality, same order as main.cpp does it for PlatformIO, only
 * without the watchdog that ESPHome already owns
*/
void DivoomComponent::setup() {
    SettingsHandler::setup();
    BluetoothHandler::setup();
    WifiHandler::setup();
    BaseInput::setup();
    BaseOutput::setup();
}

/**
 * loop functionality
*/
void DivoomComponent::loop() {
    if (failPending) {
        ESP_LOGE(TAG, "restarting: %s", failReason);
        App.safe_reboot();
        return;
    }

    BluetoothHandler::loop();
    WifiHandler::loop();
    BaseInput::loop();
}

void DivoomComponent::dump_config() {
    ESP_LOGCONFIG(TAG, "Divoom:");
    ESP_LOGCONFIG(TAG, "  Bluetooth Name: %s", SettingsHandler::bluetoothName.c_str());
    ESP_LOGCONFIG(TAG, "  TCP Port: %u", SettingsHandler::tcpPort);
    ESP_LOGCONFIG(TAG, "  TCP Max Clients: %d", TCP_MAX);
    if (!SettingsHandler::mqttHost.isEmpty()) {
        ESP_LOGCONFIG(TAG, "  MQTT Topic: %s", SettingsHandler::mqttTopic.c_str());
    }
}

void DivoomComponent::set_bluetooth_name(const char *name) { SettingsHandler::bluetoothName = name; }
void DivoomComponent::set_tcp_port(uint16_t port) { SettingsHandler::tcpPort = port; }
void DivoomComponent::set_mqtt_host(const char *host) { SettingsHandler::mqttHost = host; }
void DivoomComponent::set_mqtt_port(uint16_t port) { SettingsHandler::mqttPort = port; }
void DivoomComponent::set_mqtt_user(const char *user) { SettingsHandler::mqttUser = user; }
void DivoomComponent::set_mqtt_pass(const char *pass) { SettingsHandler::mqttPass = pass; }
void DivoomComponent::set_mqtt_client(const char *client) { SettingsHandler::mqttClient = client; }
void DivoomComponent::set_mqtt_topic(const char *topic) { SettingsHandler::mqttTopic = topic; }

}  // namespace divoom
}  // namespace esphome

/**
 * mark the component as failure, that needs a restart (like BT discovery stuck).
*/
void divoomFail(const char *reason) {
    esphome::divoom::failReason = reason;
    esphome::divoom::failPending = true;
}

#endif
