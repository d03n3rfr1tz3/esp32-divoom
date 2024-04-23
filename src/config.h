#ifndef _CONFIG_H
    #define _CONFIG_H
    
    #define BLUETOOTH_NAME   "ESP32-Divoom"

    #define WIFI_NAME        "ESP32-Divoom"

    #define WIFISSID1        ""
    #define WIFIPASS1        ""

    #define WIFISSID2        ""
    #define WIFIPASS2        ""

    #define TCP_PORT         7777
    #define TCP_MAX          5
#endif

#if __has_include("config_local.h")
# include "config_local.h"
#endif