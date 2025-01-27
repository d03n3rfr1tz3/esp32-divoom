
#ifndef _UTIL_H
    #define _UTIL_H

    #include "Arduino.h"

    #if defined ( BOARD_HAS_PSRAM )
        #define MALLOC         ps_malloc            /** @brief malloac from PSRAM */
        #define CALLOC         ps_calloc            /** @brief calloc from PSRAM */
        #define REALLOC        ps_realloc           /** @brief realloc from PSRAM */
    #else
        #define MALLOC         malloc               /** @brief malloac from normal heap */
        #define CALLOC         calloc               /** @brief calloc from normal heap */
        #define REALLOC        realloc              /** @brief realloc from normal heap */
    #endif

    typedef struct {
        uint8_t data[CONFIG_LWIP_TCP_MSS];
        size_t size;
    } data_packet_t;

    unsigned long getElapsed(unsigned long compareMillis);
#endif