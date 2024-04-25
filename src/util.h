
#ifndef _UTIL_H
    #define _UTIL_H

    #include "Arduino.h"

    typedef struct {
        uint8_t data[CONFIG_LWIP_TCP_MSS];
        size_t size;
    } data_packet_t;

    unsigned long getElapsed(unsigned long compareMillis);
#endif