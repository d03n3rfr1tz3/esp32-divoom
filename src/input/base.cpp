
#include "base.h"

/**
 * setup functionality
*/
void BaseInput::setup() {
    TcpInput::setup();
    MqttInput::setup();
    SerialInput::setup();
}

/**
 * loop functionality
*/
void BaseInput::loop() {
    TcpInput::loop();
    MqttInput::loop();
    SerialInput::loop();
}

/**
 * the forward channel for a bluetooth connection
*/
void BaseInput::forward(const char *address, uint16_t port) {
    TcpInput::forward(address, port);
    MqttInput::forward(address, port);
    SerialInput::forward(address, port);
}

/**
 * the forward channel for bluetooth data
*/
void BaseInput::forward(const uint8_t *buffer, size_t size) {
    TcpInput::forward(buffer, size);
    MqttInput::forward(buffer, size);
    SerialInput::forward(buffer, size);
}

/**
 * the backward channel for bluetooth data
*/
void BaseInput::backward(const uint8_t *buffer, size_t size) {
    TcpInput::backward(buffer, size);
    MqttInput::backward(buffer, size);
    SerialInput::backward(buffer, size);
}

/**
 * the channel for an advertised bluetooth device
*/
void BaseInput::advertise(const uint8_t* address, const char* name, size_t size) {
    TcpInput::advertise(address, name, size);
    MqttInput::advertise(address, name, size);
    SerialInput::advertise(address, name, size);
}
