
#ifndef _OUTPUT_H
    #define _OUTPUT_H
    
    #include "Arduino.h"

    #include "serial.h"
    #include "bluetooth.h"

    class BaseOutput {
        public:
            static void forward(const uint8_t *buffer, size_t size);
    };
#endif