#pragma once

#include <stdio.h>
#include <string>

#include "divoom/divoom.h"

/** injected by test/inject_project_dir.py; relative paths are the fallback */
#ifndef PROJECT_DIR
    #define PROJECT_DIR "."
#endif

inline std::string hexdump(const uint8_t* data, size_t size) {
    std::string result;
    char byte[3];

    for (size_t i = 0; i < size; i++) {
        if (i > 0) result += ' ';
        snprintf(byte, sizeof(byte), "%02x", data[i]);
        result += byte;
    }

    return result;
}

/** one hex-dumped wire message per line, mirroring support.py::format_golden */
inline std::string format_commands(const data_commands_t* commands) {
    std::string result;

    for (uint8_t i = 0; i < commands->count; i++) {
        result += hexdump(commands->command[i].data, commands->command[i].size);
        result += '\n';
    }

    return result;
}

inline std::string fixture_path(const char* dir, const char* name, bool relative) {
    std::string result = relative ? std::string("test/")
                                  : std::string(PROJECT_DIR) + "/test/";
    return result + dir + "/" + name + ".txt";
}

inline std::string golden_path(const char* name, bool relative) {
    return fixture_path("goldens", name, relative);
}

/**
 * Binary mode is required: in text mode Windows translates '\n' to "\r\n", which
 * would make the fixtures differ from the ones the Linux CI reads and writes.
*/
inline bool read_fixture(const char* dir, const char* name, std::string& content) {
    for (int relative = 0; relative <= 1; relative++) {
        FILE* file = fopen(fixture_path(dir, name, relative != 0).c_str(), "rb");
        if (file == NULL) continue;

        char chunk[512];
        size_t read = 0;
        content.clear();
        while ((read = fread(chunk, 1, sizeof(chunk), file)) > 0) {
            content.append(chunk, read);
        }

        fclose(file);
        return true;
    }

    return false;
}

inline bool read_golden(const char* name, std::string& content) {
    return read_fixture("goldens", name, content);
}

/** recorded hass-divoom output, one hex-dumped frame per line */
inline bool read_stream(const char* name, std::string& content) {
    return read_fixture("streams", name, content);
}

inline bool write_golden(const char* name, const std::string& content) {
    for (int relative = 0; relative <= 1; relative++) {
        FILE* file = fopen(golden_path(name, relative != 0).c_str(), "wb");
        if (file == NULL) continue;

        size_t written = fwrite(content.data(), 1, content.size(), file);
        fclose(file);
        return written == content.size();
    }

    return false;
}
