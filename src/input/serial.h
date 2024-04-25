
#ifndef _INPUT_SERIAL_H
    #define _INPUT_SERIAL_H

    #include "Arduino.h"

    class SerialInput {
        public:
            static void setup();
            static void loop();
            
            static void forward(const char *address, uint16_t port);
            static void forward(const uint8_t *buffer, size_t size);
            static void backward(const uint8_t *buffer, size_t size);
            static void advertise(const uint8_t* address, const char* name, size_t size);
        
        private:
            static void parse(char *buffer, size_t size);
            static void write(const char* direction, const uint8_t *buffer, size_t size);
    };
#endif