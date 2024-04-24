
#ifndef _BLUETOOTH_H
    #define _BLUETOOTH_H

    #include "config.h"
    
    #include "Arduino.h"
    #include "BluetoothSerial.h"
    #include "ESPmDNS.h"

    class BluetoothHandler {
        public:
            static void setup(void);
            static void loop(void);
            static bool check(void);

            static bool connect(BTAddress address, uint16_t channel, const char *pin);
            static bool connect(BTAddress address, uint16_t channel);
            static bool disconnect(void);

            static size_t send(const uint8_t *buffer, size_t size);

        private:
            inline static bool isConnected;
            inline static bool isScanning;
            inline static unsigned long timer;
            inline static BluetoothSerial serialBT;

            static void discovered(BTAdvertisedDevice* device);
            static void received(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
    };
#endif