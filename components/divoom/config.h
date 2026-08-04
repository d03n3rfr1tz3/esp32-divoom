#ifndef _CONFIG_H
    #define _CONFIG_H

    #ifndef LED_BUILTIN
        #define LED_BUILTIN  0
    #endif

    #ifndef DIVOOM_STATUS_LED
        #define DIVOOM_STATUS_LED LED_BUILTIN
    #endif


    /* Bluetooth Configuration */
    #ifndef BLUETOOTH_RETRY
        #define BLUETOOTH_RETRY  3
    #endif
    #ifndef BLUETOOTH_FILTER
        #define BLUETOOTH_FILTER true
    #endif

    /* TCP Configuration */
    #ifndef TCP_PORT
        #define TCP_PORT         7777
    #endif
    #ifndef TCP_MAX
        #define TCP_MAX          3
    #endif

        /* PlatformIO Configuration */
    #ifndef DIVOOM_PLATFORM_ESPHOME

        /* Bluetooth Configuration */
        #ifndef BLUETOOTH_NAME
            #define BLUETOOTH_NAME   "ESP32-Divoom"
        #endif

        /* WiFi Configuration */
        #ifndef WIFI_NAME
            #define WIFI_NAME        "ESP32-Divoom"
        #endif
        #ifndef WIFI_RETRY
            #define WIFI_RETRY       5
        #endif

        #ifndef WIFISSID1
            #define WIFISSID1        ""
        #endif
        #ifndef WIFIPASS1
            #define WIFIPASS1        ""
        #endif

        #ifndef WIFISSID2
            #define WIFISSID2        ""
        #endif
        #ifndef WIFIPASS2
            #define WIFIPASS2        ""
        #endif

        /* Serial Output Configuration */
        #ifndef SERIAL_OUT_RX
            #define SERIAL_OUT_RX    -1
        #endif
        #ifndef SERIAL_OUT_TX
            #define SERIAL_OUT_TX    -1
        #endif

        /* MQTT Configuration */
        #ifndef MQTT_CLIENT
            #define MQTT_CLIENT      "ESP32-Divoom"
        #endif
        #ifndef MQTT_HOST
            #define MQTT_HOST        ""
        #endif
        #ifndef MQTT_PORT
            #define MQTT_PORT        1883
        #endif
        #ifndef MQTT_USER
            #define MQTT_USER        ""
        #endif
        #ifndef MQTT_PASS
            #define MQTT_PASS        ""
        #endif
        #ifndef MQTT_TOPIC
            #define MQTT_TOPIC       "divoom/%s"
        #endif
    #endif
#endif

/* PlatformIO Configuration */
#ifndef DIVOOM_PLATFORM_ESPHOME
    #if __has_include("config_local.h")
    #   include "config_local.h"
    #endif
#endif
