
#include "input_mqtt_backend.h"

#include "AsyncMqttClient.h"

#include "input_mqtt.h"

static AsyncMqttClient mqttClient;

/**
 * adapters from the AsyncMqttClient callbacks to MqttInput
*/
static void onConnect(bool sessionPresent) {
    MqttInput::connected(sessionPresent);
}

static void onDisconnect(AsyncMqttClientDisconnectReason reason) {
    MqttInput::disconnected();
}

static void onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    MqttInput::message(topic, payload, len, index, total);
}

/**
 * setup functionality
*/
void MqttBackend::setup(const char *clientId, const char *user, const char *pass, const char *host, uint16_t port, const char *willTopic) {
    mqttClient.setKeepAlive(10);
    mqttClient.setClientId(clientId);
    mqttClient.setCredentials(user, pass);
    mqttClient.setServer(host, port);
    mqttClient.setWill(willTopic, 1, true, "offline");

    mqttClient.onConnect(onConnect);
    mqttClient.onDisconnect(onDisconnect);
    mqttClient.onMessage(onMessage);
}

void MqttBackend::connect() {
    mqttClient.connect();
}

bool MqttBackend::connected() {
    return mqttClient.connected();
}

void MqttBackend::publish(const char *topic, uint8_t qos, bool retain, const char *payload) {
    mqttClient.publish(topic, qos, retain, payload);
}

void MqttBackend::subscribe(const char *topic, uint8_t qos) {
    mqttClient.subscribe(topic, qos);
}
