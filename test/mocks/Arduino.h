#pragma once

#include <stdint.h>   // uint8_t, int8_t, uint16_t, uint32_t
#include <stddef.h>   // size_t
#include <string.h>   // strlen, strncmp, strtok, strstr, memcpy
#include <stdlib.h>   // strtoul, strtol, strtof, strtod, NULL
#include <math.h>     // round() in Divoom::send_weather
#include <string>     // std::string in send_datetime/show_countdown/show_alarm/show_memorial

/** used by util.cpp::getElapsed, implemented in test/arduino_stub.cpp */
unsigned long millis();

/** drives millis() in tests, writable directly */
extern unsigned long fake_millis;
