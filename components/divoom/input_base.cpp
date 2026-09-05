
#include "input_base.h"

/**
 * setup functionality
*/
void BaseInput::setup() {
    if (DIVOOM_STATUS_LED > 0) pinMode(DIVOOM_STATUS_LED, OUTPUT);

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
    if (DIVOOM_STATUS_LED > 0) digitalWrite(DIVOOM_STATUS_LED, HIGH);

    TcpInput::forward(buffer, size);
    MqttInput::forward(buffer, size);
    SerialInput::forward(buffer, size);

    if (DIVOOM_STATUS_LED > 0) digitalWrite(DIVOOM_STATUS_LED, LOW);
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
void BaseInput::advertise(const uint8_t* address, const char* name, size_t size, bool supported) {
    if (DIVOOM_STATUS_LED > 0) digitalWrite(DIVOOM_STATUS_LED, HIGH);

    TcpInput::advertise(address, name, size, supported);
    MqttInput::advertise(address, name, size, supported);
    SerialInput::advertise(address, name, size, supported);
    
    if (DIVOOM_STATUS_LED > 0) digitalWrite(DIVOOM_STATUS_LED, LOW);
}
