
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "input_serial.h"

/**
 * ESPHome owns the serial port for its own logging, so there is no serial input
*/
void SerialInput::setup() {}
void SerialInput::loop() {}

void SerialInput::forward(const char *address, uint16_t port) {}
void SerialInput::forward(const uint8_t *buffer, size_t size) {}
void SerialInput::backward(const uint8_t *buffer, size_t size) {}
void SerialInput::advertise(const uint8_t* address, const char* name, size_t size, bool supported) {}

#endif
