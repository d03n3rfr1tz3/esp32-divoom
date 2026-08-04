
#ifndef _SETTINGS_H
    #define _SETTINGS_H

    #include "config.h"
    #include "validate.h"

    #include "Arduino.h"

    class SettingsHandler {
        public:
            static void setup(void);

            inline static String wifiSsid1;
            inline static String wifiPass1;
            inline static String wifiSsid2;
            inline static String wifiPass2;
            inline static String wifiName;
            inline static String bluetoothName;

            inline static String mqttHost;
            inline static String mqttUser;
            inline static String mqttPass;
            inline static String mqttClient;
            inline static String mqttTopic;
            inline static uint16_t mqttPort;

            // not backed by NVS; the PlatformIO build keeps the compile time
            // default, the ESPHome variant sets it from its YAML
            inline static uint16_t tcpPort = TCP_PORT;
    };
#endif
