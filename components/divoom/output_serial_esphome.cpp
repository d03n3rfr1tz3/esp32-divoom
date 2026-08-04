
#ifdef DIVOOM_PLATFORM_ESPHOME

#include "output_serial.h"

/**
 * ESPHome owns the serial port for its own logging, so there is no serial output
*/
void SerialOutput::setup() {}
void SerialOutput::forward(const uint8_t *buffer, size_t size) {}
void SerialOutput::backward(const uint8_t *buffer, size_t size) {}

#endif
