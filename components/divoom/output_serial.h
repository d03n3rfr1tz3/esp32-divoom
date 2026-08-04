
#ifndef _OUTPUT_SERIAL_H
    #define _OUTPUT_SERIAL_H

    #include "Arduino.h"

    class SerialOutput {
        public:
            static void setup();
            static void forward(const uint8_t *buffer, size_t size);
            static void backward(const uint8_t *buffer, size_t size);
    };
#endif