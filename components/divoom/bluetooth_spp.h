
#ifndef _BLUETOOTH_SPP_H
    #define _BLUETOOTH_SPP_H

    #include <string>
    #include <vector>

    #include "esp_spp_api.h"
    #include "bluetooth_address.h"

    struct BluetoothDevice {
        BTAddress address;
        std::string name;
    };

    class BluetoothSpp {
        public:
            static bool begin(const char *name);
            static bool connect(const BTAddress &address, uint16_t channel);
            static bool connected(uint32_t timeout);
            static bool disconnect(void);
            static void setPin(const char *pin);

            static std::vector<BluetoothDevice> discover(int timeout);
            static void discoverStop(void);

            static size_t write(const uint8_t *buffer, size_t size);
            static void onEvent(esp_spp_cb_t callback);
    };
#endif
