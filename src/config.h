#ifndef _CONFIG_H
    #define _CONFIG_H
    
    /* Bluetooth Configuration */
    #define BLUETOOTH_NAME   "ESP32-Divoom"

    /* WiFi Configuration */
    #define WIFI_NAME        "ESP32-Divoom"

    #define WIFISSID1        ""
    #define WIFIPASS1        ""

    #define WIFISSID2        ""
    #define WIFIPASS2        ""

    /* TCP Configuration */
    #define TCP_PORT         7777
    #define TCP_MAX          3

    /* MQTT Configuration */
    #define MQTT_CLIENT      "ESP32-Divoom"
    #define MQTT_HOST        ""
    #define MQTT_PORT        1883
    #define MQTT_USER        ""
    #define MQTT_PASS        ""
    #define MQTT_TOPIC       "divoom/%s"
#endif

#if __has_include("config_local.h")
# include "config_local.h"
#endif