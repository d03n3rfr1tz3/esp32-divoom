#pragma once

struct DivoomCase {
    const char* name;
    const char* command;
};

/** MODE strings as accepted by Divoom::parseMode; argument order follows divoom.cpp */
static const DivoomCase CASES[] = {
    { "brightness",      "brightness 42" },
    { "volume",          "volume 60" },
    { "playstate",       "playstate 1" },
    { "weather_celsius", "weather 22°C 3" },
    { "datetime",        "datetime 2024-01-02 03:04:05" },
    { "clock",           "clock 3 1 1 1 1 0a141e 1" },
    { "countdown",       "countdown 1 00:30" },
    { "noise",           "noise 1" },
    { "timer",           "timer 5" },
};

static const size_t CASE_COUNT = sizeof(CASES) / sizeof(CASES[0]);
