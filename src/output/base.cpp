
#include "base.h"

/**
 * the forward channel for bluetooth data
*/
void BaseOutput::forward(const uint8_t *buffer, size_t size) {
    BluetoothOutput::forward(buffer, size);
    SerialOutput::forward(buffer, size);
}
