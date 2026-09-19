
#ifndef _BLUETOOTH_ADDRESS_H
    #define _BLUETOOTH_ADDRESS_H

    #include <cstdio>
    #include <cstring>
    #include <string>

    #include "esp_gap_bt_api.h"

    class BTAddress {
        public:
            BTAddress() { memset(address, 0, ESP_BD_ADDR_LEN); }
            BTAddress(esp_bd_addr_t native) { memcpy(address, native, ESP_BD_ADDR_LEN); }

            const uint8_t *getNative() const { return address; }

            std::string toString() const {
                char buffer[18];
                snprintf(buffer, sizeof(buffer), "%02x:%02x:%02x:%02x:%02x:%02x", address[0], address[1], address[2], address[3], address[4], address[5]);
                return buffer;
            }

            explicit operator bool() const {
                for (size_t i = 0; i < ESP_BD_ADDR_LEN; i++)
                    if (address[i]) return true;
                return false;
            }

        private:
            esp_bd_addr_t address;
    };
#endif
