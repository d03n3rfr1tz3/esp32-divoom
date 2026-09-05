
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "input_mqtt_backend.h"
#include "input_mqtt.h"

#include "esphome/core/defines.h"

#ifdef USE_MQTT
#include "esphome/components/mqtt/mqtt_client.h"

using esphome::mqtt::global_mqtt_client;
using esphome::mqtt::MQTTClientDisconnectReason;

/**
 * setup functionality, where only the event handlers are ours; connection,
 * credentials and last will come from the mqtt block of the YAML
*/
void MqttBackend::setup(const char *clientId, const char *user, const char *pass, const char *host, uint16_t port, const char *willTopic) {
    global_mqtt_client->set_on_connect([](bool sessionPresent) { MqttInput::connected(sessionPresent); });
    global_mqtt_client->set_on_disconnect([](MQTTClientDisconnectReason reason) { MqttInput::disconnected(); });
}

/**
 * connects, which ESPHome already does on its own
*/
void MqttBackend::connect() {}

bool MqttBackend::connected() {
    return global_mqtt_client->is_connected();
}

void MqttBackend::publish(const char *topic, uint8_t qos, bool retain, const char *payload) {
    global_mqtt_client->publish(topic, payload, strlen(payload), qos, retain);
}

void MqttBackend::subscribe(const char *topic, uint8_t qos) {
    global_mqtt_client->subscribe(topic, [](const std::string &topic, const std::string &payload) {
        MqttInput::message(topic.c_str(), payload.c_str(), payload.size(), 0, payload.size());
    }, qos);
}

#else

// without a mqtt block in the YAML there is no client to talk to
void MqttBackend::setup(const char *clientId, const char *user, const char *pass, const char *host, uint16_t port, const char *willTopic) {}
void MqttBackend::connect() {}
bool MqttBackend::connected() { return false; }
void MqttBackend::publish(const char *topic, uint8_t qos, bool retain, const char *payload) {}
void MqttBackend::subscribe(const char *topic, uint8_t qos) {}

#endif
#endif
