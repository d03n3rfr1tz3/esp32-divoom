

#include "input_mqtt.h"

#include "settings.h"

#include "hardware_bluetoothctl.h"
#include "hardware_wifictl.h"

#include "input_base.h"
#include "output_base.h"

#include "divoom.h"

MqttInput::MqttInput() {
    timer = millis();
}

static inline bool mqttEnabled(void) {
    return !SettingsHandler::mqttHost.isEmpty();
}

/**
 * setup functionality
*/
void MqttInput::setup() {
    if (!mqttEnabled()) return;

    const char *topicPattern = SettingsHandler::mqttTopic.c_str();
    snprintf(topicState, sizeof( topicState ), topicPattern, "proxy");
    snprintf(topicHeap, sizeof( topicHeap ), topicPattern, "heap");
    snprintf(topicBluetooth, sizeof( topicBluetooth ), topicPattern, "bluetooth");
    snprintf(topicCommand, sizeof( topicCommand ), topicPattern, "command");

    MqttBackend::setup(SettingsHandler::mqttClient.c_str(),
                       SettingsHandler::mqttUser.c_str(), SettingsHandler::mqttPass.c_str(),
                       SettingsHandler::mqttHost.c_str(), SettingsHandler::mqttPort, topicState);
}

/**
 * loop functionality
*/
void MqttInput::loop() {
    if (!mqttEnabled()) return;

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
    if (!mqttEnabled()) return false;
    if (!wasWifiConnected) return false;

    if (MqttBackend::connected())
    {
        isConnected = true;
        return true;
    }
    else
    {
        MqttBackend::connect();
        return isConnected = MqttBackend::connected();
    }
}

/**
 * updates some basic values in MQTT
*/
void MqttInput::update(void) {
    if (!mqttEnabled()) return;
    if (!isConnected) return;

    MqttBackend::publish(topicState, 1, true, "online");
    MqttBackend::publish(topicBluetooth, 1, false, BluetoothHandler::check() ? "connected" : "disconnected");

    char heapPayload[22];
    snprintf(heapPayload, sizeof(heapPayload), "%lu/%lu", (unsigned long)ESP.getFreeHeap(), (unsigned long)ESP.getHeapSize());
    MqttBackend::publish(topicHeap, 0, false, heapPayload);
}

/**
 * the forward channel for a bluetooth connection
*/
void MqttInput::forward(const char *address, uint16_t port) {
    if (!mqttEnabled()) return;
    if (!isConnected) return;

    MqttBackend::publish(topicBluetooth, 1, false, port > 0 ? "connecting" : "disconnected");
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
    if (!mqttEnabled()) return;
    if (!isConnected) return;

    const char *payload = BluetoothHandler::check() ? "connected" : "disconnected";
    if (size == 1 && buffer[0] == 0x69) payload = "connecting";
    if (size == 1 && buffer[0] == 0x96) payload = "disconnected";
    if (size > 2 && buffer[0] == 0x01 && buffer[size - 1] == 0x02) payload = "connected";

    MqttBackend::publish(topicBluetooth, 1, false, payload);
}

/**
 * the channel for an advertised bluetooth device
*/
void MqttInput::advertise(const uint8_t* address, const char* name, size_t size, bool supported) {
    if (!mqttEnabled()) return;
    if (!isConnected) return;
    
    char topicAddress[strlen("advertise/") + 18 + 1];
    snprintf(topicAddress, sizeof( topicAddress ), "advertise/%02X:%02X:%02X:%02X:%02X:%02X", address[0], address[1], address[2], address[3], address[4], address[5]);

    char topicAdvertise[VALIDATE_TOPIC_MAX + sizeof( topicAddress )];
    snprintf(topicAdvertise, sizeof( topicAdvertise ), SettingsHandler::mqttTopic.c_str(), topicAddress);
    MqttBackend::publish(topicAdvertise, 0, false, name);
}

/**
 * onConnected event handler
*/
void MqttInput::connected(bool sessionPresent) {
    isConnected = true;

    MqttBackend::subscribe(topicCommand, 0);
    MqttBackend::publish(topicState, 1, true, "online");

    update();
}

/**
 * onDisconnected event handler
*/
void MqttInput::disconnected() {
    isConnected = false;
}

/**
 * onMessage event handler
*/
void MqttInput::message(const char* topic, const char* payload, size_t len, size_t index, size_t total) {
    if (strcmp(topic, topicCommand) != 0) return;
    if (total >= sizeof(messageBuffer) || index + len > total) return;

    memcpy(messageBuffer + index, payload, len);
    if (index + len < total) return;

    messageBuffer[total] = '\0';
    parse(topic, messageBuffer, total);
}

/**
 * the parser for incoming data
*/
void MqttInput::parse(const char* topic, char* payload, size_t size) {
    if (strcmp(topic, topicCommand) != 0) return;

    char *buffer = payload;

    // recognize a connect statement and pass it into Bluetooth handler
    if (size > strlen("CONNECT ") && strncmp("CONNECT ", (const char*)buffer, strlen("CONNECT ")) == 0) {
        size_t offset = strlen("CONNECT ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        size_t index = 0;
        esp_bd_addr_t bytes = { 0 };
        uint16_t port = 1;

        char *token = strtok(content, ":");
        while (token != NULL && index < ESP_BD_ADDR_LEN) {
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
        esp_bd_addr_t bytes = { 0 };
        uint16_t port = 0;

        char *token = strtok(content, ":");
        while (token != NULL && index < ESP_BD_ADDR_LEN) {
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
            if (i > 0) delay(25);
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
        uint8_t bytes[sizeof(data_command_t::data)];

        char *token = strtok(content, " ");
        while (token != NULL && index < sizeof(bytes)) {
            bytes[index++] = strtoul(token, NULL, 16);
            token = strtok(NULL, " ");
        }

        BaseInput::forward(bytes, index);
        BaseOutput::forward(bytes, index);
    }
}