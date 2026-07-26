
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

    /** a 32x32 image with a full 1024 color palette measures 4373 bytes */
    #define FRAME_BUFFER_SIZE  4608

    typedef struct {
        uint8_t data[CONFIG_LWIP_TCP_MSS];
        size_t size;
    } data_packet_t;

    typedef struct {
        uint8_t data[FRAME_BUFFER_SIZE];
        size_t size;
    } frame_stream_t;

    typedef void (*frame_handler_t)(const uint8_t *buffer, size_t size);

    unsigned long getElapsed(unsigned long compareMillis);

    size_t getFrameSize(const uint8_t *buffer, size_t size);
    bool isFramePending(const uint8_t *buffer, size_t size);
    void appendFrames(frame_stream_t *stream, const uint8_t *buffer, size_t size, frame_handler_t handler);
#endif