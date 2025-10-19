

#include "mqtt.h"

#include "hardware/bluetoothctl.h"
#include "hardware/wifictl.h"

#include "input/base.h"
#include "output/base.h"

#include "divoom/divoom.h"

AsyncMqttClient mqttClient;

MqttInput::MqttInput() {
    timer = millis();
}

/**
 * setup functionality
*/
void MqttInput::setup() {
    if (strlen(MQTT_HOST) == 0) return;

    snprintf(topicState, sizeof( topicState ), MQTT_TOPIC, "proxy");
    snprintf(topicHeap, sizeof( topicHeap ), MQTT_TOPIC, "heap");
    snprintf(topicBluetooth, sizeof( topicBluetooth ), MQTT_TOPIC, "bluetooth");
    snprintf(topicCommand, sizeof( topicCommand ), MQTT_TOPIC, "command");

    mqttClient.setKeepAlive(10);
    mqttClient.setClientId(MQTT_CLIENT);
    mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setWill(topicState, 1, true, "offline");

    mqttClient.onConnect(connected);
    mqttClient.onDisconnect(disconnected);
    mqttClient.onMessage(message);
}

/**
 * loop functionality
*/
void MqttInput::loop() {
    if (strlen(MQTT_HOST) == 0) return;

    bool isWifiConnected = WifiHandler::check(true);
    if (getElapsed(timer) > 15000 || (isWifiConnected && !wasWifiConnected)) {
        if (!isWifiConnected) isConnected = false;
        wasWifiConnected = isWifiConnected;
        timer = millis();

        if (check()) {
            update();
        }
    }
}

/**
 * checks connection and keeps it up
*/
bool MqttInput::check(void) {
    if (strlen(MQTT_HOST) == 0) return false;
    if (!wasWifiConnected) return false;

    if (mqttClient.connected())
    {
        isConnected = true;
        return true;
    }
    else
    {
        mqttClient.connect();
        return isConnected = mqttClient.connected();
    }
}

/**
 * updates some basic values in MQTT
*/
void MqttInput::update(void) {
    if (strlen(MQTT_HOST) == 0) return;
    if (!isConnected) return;

    mqttClient.publish(topicBluetooth, 1, false, BluetoothHandler::check() ? "connected" : "disconnected");
    
    char heapPayload[22];
    snprintf(heapPayload, sizeof(heapPayload), "%d/%d", ESP.getFreeHeap(), ESP.getHeapSize());
    mqttClient.publish(topicHeap, 0, false, heapPayload);
}

/**
 * the forward channel for a bluetooth connection
*/
void MqttInput::forward(const char *address, uint16_t port) {
    if (strlen(MQTT_HOST) == 0) return;
    if (!isConnected) return;

    mqttClient.publish(topicBluetooth, 1, false, port > 0 ? "connecting" : "disconnected");
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
    if (strlen(MQTT_HOST) == 0) return;
    if (!isConnected) return;

    const char *payload = BluetoothHandler::check() ? "connected" : "disconnected";
    if (size == 1 && buffer[0] == 0x69) payload = "connecting";
    if (size == 1 && buffer[0] == 0x96) payload = "disconnected";
    if (size > 2 && buffer[0] == 0x01 && buffer[size - 1] == 0x02) payload = "connected";

    mqttClient.publish(topicBluetooth, 1, false, payload);
}

/**
 * the channel for an advertised bluetooth device
*/
void MqttInput::advertise(const uint8_t* address, const char* name, size_t size, bool supported) {
    if (strlen(MQTT_HOST) == 0) return;
    if (!isConnected) return;
    
    size_t j = 0;
    char topicAddress[strlen("advertise/") + 18 + 1];
    snprintf(topicAddress, sizeof( topicAddress ), "advertise/%02X:%02X:%02X:%02X:%02X:%02X", address[0], address[1], address[2], address[3], address[4], address[5]);

    char topicAdvertise[strlen(MQTT_TOPIC) + strlen("advertise/") + strlen(topicAddress)];
    snprintf(topicAdvertise, sizeof( topicAdvertise ), MQTT_TOPIC, topicAddress);
    mqttClient.publish(topicAdvertise, 0, false, name);
}

/**
 * onConnected event handler
*/
void MqttInput::connected(bool sessionPresent) {
    isConnected = true;

    mqttClient.subscribe(topicCommand, 0);
    mqttClient.publish(topicState, 1, true, "online");

    update();
}

/**
 * onDisconnected event handler
*/
void MqttInput::disconnected(AsyncMqttClientDisconnectReason reason) {
    isConnected = false;
}

/**
 * onMessage event handler
*/
void MqttInput::message(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    parse(topic, payload + index, len);
}

/**
 * the parser for incoming data
*/
void MqttInput::parse(char* topic, char* payload, size_t size) {
    if (strncmp(topic, topicCommand, strlen(topic)) != 0) return;
    
    char *buffer = payload;
    buffer[size] = '\0';

    // recognize a connect statement and pass it into Bluetooth handler
    if (size > strlen("CONNECT ") && strncmp("CONNECT ", (const char*)buffer, strlen("CONNECT ")) == 0) {
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
    if (size > strlen("DISCONNECT ") && strncmp("DISCONNECT ", (const char*)buffer, strlen("DISCONNECT ")) == 0) {
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
    if (size > strlen("MODE ") && strncmp("MODE ", (const char*)buffer, strlen("MODE ")) == 0) {
        size_t offset = strlen("MODE ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        Divoom divoom = Divoom();
        data_commands_t *commands = divoom.parseMode(content, size);

        for (size_t i = 0; i < commands->count; i++)
        {
            if (i > 0) sleep(25);
            data_command_t command = commands->command[i];
            BaseInput::forward(command.data, command.size);
            BaseOutput::forward(command.data, command.size);
        }
    }

    // recognize a raw statement and pass it into Output handlers
    if (size > strlen("SEND ") && strncmp("SEND ", (const char*)buffer, strlen("SEND ")) == 0) {
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