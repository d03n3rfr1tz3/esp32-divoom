#pragma once

struct DivoomCase {
    const char* name;
    const char* command;
};

/** MODE strings as accepted by Divoom::parseMode; argument order follows divoom.cpp */
static const DivoomCase CASES[] = {
    { "brightness",           "brightness 42" },
    { "volume",               "volume 60" },
    { "keyboard_up",          "keyboard 1" },
    { "keyboard_off",         "keyboard 0" },
    { "keyboard_down",        "keyboard -1" },
    { "playstate",            "playstate 1" },
    { "weather_celsius",      "weather 22°C 3" },
    { "weather_fahrenheit",   "weather 72°F 3" },
    // no unit suffix: send_weather skips the second frame
    { "weather_plain",        "weather 22 3" },
    { "datetime",             "datetime 2024-01-02 03:04:05" },
    { "clock",                "clock 3 1 1 1 1 0a141e 1" },
    { "light",                "light 77 ff0080 1" },
    { "effects",              "effects 2" },
    { "visualization",        "visualization 1" },
    // show_design emits the view frame plus a design frame for 0 <= value <= 3
    { "design",               "design 2" },
    // version 0 selects the view, version 1 the tool variant
    { "scoreboard_view",      "scoreboard 0 12 34" },
    { "scoreboard_tool",      "scoreboard 1 12 34" },
    { "lyrics",               "lyrics" },
    { "countdown",            "countdown 1 00:30" },
    { "noise",                "noise 1" },
    { "timer",                "timer 5" },
    { "alarm",                "alarm 1 07:30 1010100 1 1 101.1 80" },
    // parseMode reassembles the one space strtok consumed, see test_helpers.cpp
    { "memorial",             "memorial 2 2024-05-06 12:00:00 Test" },
    { "memorial_two_words",   "memorial 2 2024-05-06 12:00:00 Two Words" },
    { "radio",                "radio 1 101.3" },
    { "sleep",                "sleep 1 45 1 99.5 50 090909 60" },
    { "game",                 "game 2" },
    // send_gamecontrol emits two frames for a press, one for a release
    { "gamecontrol_press",    "gamecontrol 1" },
    { "gamecontrol_release",  "gamecontrol 0" },
};

static const size_t CASE_COUNT = sizeof(CASES) / sizeof(CASES[0]);
