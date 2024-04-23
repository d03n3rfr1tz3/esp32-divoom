
#include "serial.h"

#include "input/base.h"
#include "output/base.h"

HardwareSerial serialOut(1);

/**
 * setup functionality
*/
void SerialOutput::setup() {
    serialOut.begin(9600);
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
