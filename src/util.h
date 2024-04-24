
#ifndef _UTIL_H
    #define _UTIL_H

    #include "Arduino.h"

    typedef struct {
        const uint8_t *data;
        size_t size;
    } data_packet_t;

    unsigned long getElapsed(unsigned long compareMillis);
#endif