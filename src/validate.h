
#ifndef _VALIDATE_H
    #define _VALIDATE_H

    #include "string.h"

    #define VALIDATE_TOPIC_MAX     40
    #define VALIDATE_HOSTNAME_MAX  31

    /**
     * checks that a topic pattern is safe to use as a snprintf format string,
     * which means exactly one conversion and that conversion being %s
    */
    static inline bool isValidTopicFormat(const char* topic) {
        if (topic == NULL) return false;

        size_t length = strlen(topic);
        if (length < 1 || length > VALIDATE_TOPIC_MAX) return false;

        size_t count = 0;
        for (size_t i = 0; i < length; i++) {
            if (topic[i] != '%') continue;
            if (topic[i + 1] != 's') return false;

            count++;
            i++;
        }

        return count == 1;
    }

    /**
     * checks that a name is usable as a wifi and mdns hostname
    */
    static inline bool isValidHostname(const char* name) {
        if (name == NULL) return false;

        size_t length = strlen(name);
        if (length < 1 || length > VALIDATE_HOSTNAME_MAX) return false;
        if (name[0] == '-' || name[length - 1] == '-') return false;

        for (size_t i = 0; i < length; i++) {
            char character = name[i];
            if (character >= 'A' && character <= 'Z') continue;
            if (character >= 'a' && character <= 'z') continue;
            if (character >= '0' && character <= '9') continue;
            if (character == '-') continue;

            return false;
        }

        return true;
    }
#endif
