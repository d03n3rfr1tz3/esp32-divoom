
#include "serial.h"

#include "config.h"

#include "input/base.h"
#include "output/base.h"

HardwareSerial serialOut(1);

/**
 * setup functionality
*/
void SerialOutput::setup() {
    if (SERIAL_OUT_RX < 0 || SERIAL_OUT_TX < 0) return;
    serialOut.begin(9600, SERIAL_8N1, SERIAL_OUT_RX, SERIAL_OUT_TX);
}

/**
 * the forward channel for bluetooth data
*/
void SerialOutput::forward(const uint8_t *buffer, size_t size) {
    serialOut.write(buffer, size);
    serialOut.println("");
}

/**
 * the backward channel for bluetooth data
*/
void SerialOutput::backward(const uint8_t *buffer, size_t size) {
    BaseInput::backward(buffer, size);
}
