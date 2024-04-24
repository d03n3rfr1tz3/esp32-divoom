
#include "bluetooth.h"

#include "hardware/bluetoothctl.h"

#include "input/base.h"
#include "output/base.h"

/**
 * setup functionality, that connects to or disconnects from a bluetooth device
*/
void BluetoothOutput::setup(BTAddress address, uint16_t channel) { return setup(address, channel, nullptr); };
void BluetoothOutput::setup(BTAddress address, uint16_t channel, const char *pin) {
    if (channel == 0) {
        BluetoothHandler::disconnect();
    } else {
        BluetoothHandler::connect(address, channel, pin);
    }
}

/**
 * the forward channel for bluetooth data
*/
void BluetoothOutput::forward(const uint8_t *buffer, size_t size) {
    size_t result = BluetoothHandler::send(buffer, size);
    if (result == size) return;

    if (result == 0) { // connecting
        const uint8_t data[1] = { 0x69 };
        BaseInput::backward(data, 1);
    }
    if (result == -1) { // no connection
        const uint8_t data[1] = { 0x96 };
        BaseInput::backward(data, 1);
    }
}

/**
 * the backward channel for bluetooth data
*/
void BluetoothOutput::backward(const uint8_t *buffer, size_t size) {
    BaseInput::backward(buffer, size);
}
