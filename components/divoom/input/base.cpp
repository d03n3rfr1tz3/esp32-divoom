
#include "base.h"

/**
 * setup functionality
*/
void BaseInput::setup() {
    if (LED_BUILTIN > 0) pinMode(LED_BUILTIN, OUTPUT);

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
    if (LED_BUILTIN > 0) digitalWrite(LED_BUILTIN, HIGH);

    TcpInput::forward(buffer, size);
    MqttInput::forward(buffer, size);
    SerialInput::forward(buffer, size);

    if (LED_BUILTIN > 0) digitalWrite(LED_BUILTIN, LOW);
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
    if (LED_BUILTIN > 0) digitalWrite(LED_BUILTIN, HIGH);

    TcpInput::advertise(address, name, size, supported);
    MqttInput::advertise(address, name, size, supported);
    SerialInput::advertise(address, name, size, supported);
    
    if (LED_BUILTIN > 0) digitalWrite(LED_BUILTIN, LOW);
}
