
#include "bluetooth.h"

#include "hardware/bluetoothctl.h"

#include "input/base.h"
#include "output/base.h"

/**
 * setup functionality, that connects to the given bluetooth device
*/
void BluetoothOutput::setup(BTAddress address, uint16_t channel) { return setup(address, channel, nullptr); };
void BluetoothOutput::setup(BTAddress address, uint16_t channel, const char *pin) {
    BluetoothHandler::connect(address, channel, pin);
}

/**
 * the forward channel for bluetooth data
*/
void BluetoothOutput::forward(const uint8_t *buffer, size_t size) {
    BluetoothHandler::send(buffer, size);
}

/**
 * the backward channel for bluetooth data
*/
void BluetoothOutput::backward(const uint8_t *buffer, size_t size) {
    BaseInput::backward(buffer, size);
}
