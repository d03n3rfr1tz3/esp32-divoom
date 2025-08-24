
#ifndef _WIFI_H
    #define _WIFI_H

    #include "config.h"
    
    #include "Arduino.h"
    #include "ESPmDNS.h"
    #include "WiFi.h"

    class WifiHandler {
        public:
            WifiHandler();
            static void setup(void);
            static void loop(void);
            static bool check(bool fast);
            static void connect(void);

        private:
            inline static bool isConnected;
            inline static uint8_t retryCount;
            inline static unsigned long timer;

            static void scanned(WiFiEvent_t event, WiFiEventInfo_t info);
            static void connected(WiFiEvent_t event, WiFiEventInfo_t info);
            static void disconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    };
#endif