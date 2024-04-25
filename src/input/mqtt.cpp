

#include "mqtt.h"

#include "hardware/bluetoothctl.h"
#include "hardware/wifictl.h"

#include "input/base.h"
#include "output/base.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

/**
 * setup functionality
*/
void MqttInput::setup() {
    if (strlen(MQTT_HOST) == 0) return;

    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setCallback(parse);
}

/**
 * loop functionality
*/
void MqttInput::loop() {
    if (strlen(MQTT_HOST) == 0) return;

    if (getElapsed(timer) > 10000) {
        timer = millis();

        if (check()) {
            update();
        }
    }

    mqttClient.loop();
}

/**
 * checks connection and keeps it up
*/
bool MqttInput::check(void) {
    if (strlen(MQTT_HOST) == 0) return false;

    if (mqttClient.connected())
    {
        isConnected = true;
        return true;
    }
    else
    {
        if (!WifiHandler::check(true)) return false;
        
        char topicState[strlen(MQTT_TOPIC) + strlen("proxy")];
        snprintf(topicState, sizeof( topicState ), MQTT_TOPIC, "proxy");
        mqttClient.publish(topicState, "online", true);
        
        isConnected = mqttClient.connect(MQTT_CLIENT, MQTT_USER, MQTT_PASS, topicState, 1, true, "offline");
        if (isConnected) {
            char topicCommand[strlen(MQTT_TOPIC) + strlen("command")];
            snprintf(topicCommand, sizeof( topicCommand ), MQTT_TOPIC, "command");

            mqttClient.subscribe(topicCommand);
            mqttClient.publish(topicState, "online", true);
        }

        return isConnected;
    }
}

/**
 * updates some basic values in MQTT
*/
void MqttInput::update(void) {
    if (strlen(MQTT_HOST) == 0) return;

    char topicState[strlen(MQTT_TOPIC) + strlen("proxy")];
    snprintf(topicState, sizeof( topicState ), MQTT_TOPIC, "proxy");
    mqttClient.publish(topicState, "online", true);

    char topicBluetooth[strlen(MQTT_TOPIC) + strlen("bluetooth")];
    snprintf(topicBluetooth, sizeof( topicBluetooth ), MQTT_TOPIC, "bluetooth");
    mqttClient.publish(topicBluetooth, BluetoothHandler::check(true) ? "connected" : "disconnected", false);
}

/**
 * the forward channel for a bluetooth connection
*/
void MqttInput::forward(const char *address, uint16_t port) {
    const char *payload = port > 0 ? "connecting" : "disconnected";
    char topicBluetooth[strlen(MQTT_TOPIC) + strlen("bluetooth")];
    snprintf(topicBluetooth, sizeof( topicBluetooth ), MQTT_TOPIC, "bluetooth");
    mqttClient.publish(topicBluetooth, payload, false);
}

/**
 * the forward channel for bluetooth data
*/
void MqttInput::forward(const uint8_t *buffer, size_t size) {
    // no additional handling needed
}

/**
 * the backward channel for bluetooth data
*/
void MqttInput::backward(const uint8_t *buffer, size_t size) {
    const char *payload = BluetoothHandler::check(true) ? "connected" : "disconnected";
    if (size == 1 && buffer[0] == 0x69) payload = "connecting";
    if (size == 1 && buffer[0] == 0x96) payload = "disconnected";
    if (size > 2 && buffer[0] == 0x01 && buffer[size - 1] == 0x02) payload = "connected";

    char topicBluetooth[strlen(MQTT_TOPIC) + strlen("bluetooth")];
    snprintf(topicBluetooth, sizeof( topicBluetooth ), MQTT_TOPIC, "bluetooth");
    mqttClient.publish(topicBluetooth, payload, false);
}

/**
 * the channel for an advertised bluetooth device
*/
void MqttInput::advertise(const uint8_t* address, const char* name, size_t size) {
    size_t j = 0;
    char topicAddress[strlen("advertise/") + 18 + 1];
    snprintf(topicAddress, sizeof( topicAddress ), "advertise/%02X:%02X:%02X:%02X:%02X:%02X", address[0], address[1], address[2], address[3], address[4], address[5]);

    char topicAdvertise[strlen(MQTT_TOPIC) + strlen("advertise/") + strlen(topicAddress)];
    snprintf(topicAdvertise, sizeof( topicAdvertise ), MQTT_TOPIC, topicAddress);
    mqttClient.publish(topicAdvertise, name, false);
}

/**
 * the parser for incoming data
*/
void MqttInput::parse(char* topic, uint8_t* payload, size_t size) {
    char topicCommand[strlen(MQTT_TOPIC) + strlen("command")];
    snprintf(topicCommand, sizeof( topicCommand ), MQTT_TOPIC, "command");
    if (strncmp(topic, topicCommand, strlen(topic)) != 0) return;
    char *buffer = (char*)payload;
    
    // recognize a connect statement and pass it into Bluetooth handler
    if (strncmp("CONNECT ", (const char*)buffer, strlen("CONNECT ")) == 0) {
        size_t offset = strlen("CONNECT ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        size_t index = 0;
        esp_bd_addr_t bytes;
        uint16_t port = 1;

        char *token = strtok(content, ":");
        while (token != NULL) {
            bytes[index++] = strtoul(token, NULL, 16);
            token = strtok(NULL, ":");
        }

        if (size > 18) {
            port = content[18] - '0';
        }

        BTAddress address(bytes);
        BaseInput::forward(address.toString().c_str(), port);
        BluetoothOutput::setup(address, port);
    }

    // recognize a disconnect statement and pass it into Bluetooth handler
    if (strncmp("DISCONNECT ", (const char*)buffer, strlen("DISCONNECT ")) == 0) {
        size_t offset = strlen("DISCONNECT ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        size_t index = 0;
        esp_bd_addr_t bytes;
        uint16_t port = 0;

        char *token = strtok(content, ":");
        while (token != NULL) {
            bytes[index++] = strtoul(token, NULL, 16);
            token = strtok(NULL, ":");
        }

        BTAddress address(bytes);
        BaseInput::forward(address.toString().c_str(), port);
        BluetoothOutput::setup(address, port);
    }

    // recognize a mode statement and pass it into Divoom handler
    if (strncmp("MODE ", (const char*)buffer, strlen("MODE ")) == 0) {
        size_t offset = strlen("MODE ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        //TODO: Pass the parsed content into divoom.h functions
    }

    // recognize a raw statement and pass it into Output handlers
    if (strncmp("SEND ", (const char*)buffer, strlen("SEND ")) == 0) {
        size_t offset = strlen("SEND ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        size_t index = 0;
        uint8_t bytes[size / 2];

        char *token = strtok(content, " ");
        while (token != NULL) {
            bytes[index++] = strtoul(token, NULL, 16);
            token = strtok(NULL, " ");
        }

        BaseInput::forward(bytes, index);
        BaseOutput::forward(bytes, index);
    }
}