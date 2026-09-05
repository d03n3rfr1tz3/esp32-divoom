
#ifndef _OUTPUT_BLUETOOTH_H
    #define _OUTPUT_BLUETOOTH_H
    
    #include "Arduino.h"
    #include "BluetoothSerial.h"

    class BluetoothOutput {
        public:
            static void setup(BTAddress address, uint16_t channel, const char *pin);
            static void setup(BTAddress address, uint16_t channel);
            static void forward(const uint8_t *buffer, size_t size);
            static void backward(const uint8_t *buffer, size_t size);
    };
#endif