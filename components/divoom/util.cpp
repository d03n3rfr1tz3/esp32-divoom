
#include "util.h"

/**
 * helper for calculating the elapsed time in milliseconds
*/
unsigned long getElapsed(unsigned long compareMillis) {
  unsigned long currentMillis = millis();
  unsigned long elapsedMillis = currentMillis - compareMillis;
  return elapsedMillis;
}

/**
 * checks a frame payload against its own length field and trailing checksum.
 * the checksum covers the length field as well and is 2 bytes, or 4 once it exceeds 65535
*/
static bool isFrameValid(const uint8_t *payload, size_t size) {
    if (size < 5) return false;

    size_t length = payload[0] | (payload[1] << 8);
    if (length + 2 != size && length + 4 != size) return false;

    uint32_t sum = 0;
    uint32_t checksum = 0;
    for (size_t i = 0; i < length; i++) sum += payload[i];
    for (size_t i = length; i < size; i++) checksum |= (uint32_t)payload[i] << ((i - length) * 8);

    return sum == checksum;
}

/**
 * same check for escaped payloads, where 0x01 to 0x03 arrive as 0x03 followed by 0x04 to 0x06.
 * escaping shifts every position, so the frame is decoded on the fly instead of being copied
*/
static bool isEscapedFrameValid(const uint8_t *payload, size_t size) {
    size_t index = 0;
    size_t length = 0;
    uint32_t sum = 0;
    uint32_t checksum = 0;

    for (size_t i = 0; i < size; i++) {
        uint8_t value = payload[i];
        if (value == 0x03) {
            if (++i >= size) return false;
            value = payload[i] - 0x03;
        }

        if (index == 0) length = value;
        if (index == 1) length |= value << 8;

        if (index < length) sum += value;
        else if (index < length + 4) checksum |= (uint32_t)value << ((index - length) * 8);
        index++;
    }

    if (index < 5) return false;
    if (index != length + 2 && index != length + 4) return false;

    return sum == checksum;
}

/**
 * size of the leading divoom frame, 0 while it is still incomplete or not a frame at all.
 * a frame is 0x01 | length | payload | checksum | 0x02, with length counting payload and checksum
*/
size_t getFrameSize(const uint8_t *buffer, size_t size) {
    if (size < 6 || buffer[0] != 0x01) return 0;

    size_t length = buffer[1] | (buffer[2] << 8);
    for (size_t checksumSize = 2; checksumSize <= 4; checksumSize += 2) {
        size_t frameSize = length + checksumSize + 2;
        if (frameSize > size || buffer[frameSize - 1] != 0x02) continue;
        if (isFrameValid(buffer + 1, length + checksumSize)) return frameSize;
    }

    // escaping breaks the length field, but in exchange 0x02 cannot occur inside the payload
    for (size_t i = 1; i < size; i++) {
        if (buffer[i] != 0x02) continue;
        return isEscapedFrameValid(buffer + 1, i - 1) ? i + 1 : 0;
    }

    return 0;
}

/**
 * whether the leading bytes can still grow into a complete frame,
 * telling an unfinished frame apart from data that is not framed at all
*/
bool isFramePending(const uint8_t *buffer, size_t size) {
    if (size == 0 || buffer[0] != 0x01) return false;
    if (size < 6) return true;

    size_t length = buffer[1] | (buffer[2] << 8);
    if (size < length + 6) return true;

    for (size_t i = 1; i < size; i++) {
        if (buffer[i] == 0x02) return false;
    }

    return true;
}

/**
 * appends a packet to the stream and hands every complete frame to the handler.
 * anything that is not framed is passed through unchanged, so unknown streams keep working
*/
void appendFrames(frame_stream_t *stream, const uint8_t *buffer, size_t size, frame_handler_t handler) {
    while (size > 0) {
        size_t take = sizeof(stream->data) - stream->size;
        if (take > size) take = size;

        memcpy(stream->data + stream->size, buffer, take);
        stream->size += take;
        buffer += take;
        size -= take;

        size_t offset = 0;
        size_t frameSize = 0;
        while ((frameSize = getFrameSize(stream->data + offset, stream->size - offset)) > 0) {
            handler(stream->data + offset, frameSize);
            offset += frameSize;
        }

        // keep an unfinished frame for the next packet, flush everything else to guarantee progress
        size_t rest = stream->size - offset;
        if (rest > 0 && (take == 0 || !isFramePending(stream->data + offset, rest))) {
            handler(stream->data + offset, rest);
            offset = stream->size;
            rest = 0;
        }

        memmove(stream->data, stream->data + offset, rest);
        stream->size = rest;
    }
}
