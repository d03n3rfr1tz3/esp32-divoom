#pragma once

#include "esphome/core/component.h"

namespace esphome {
namespace divoom {

/**
 * the entry point of the ESPHome variant, whose setters the codegen fills before setup
*/
class DivoomComponent : public Component {
    public:
        void setup() override;
        void loop() override;
        void dump_config() override;

        // the zeroconf service needs the mdns component, which starts at AFTER_CONNECTION
        float get_setup_priority() const override { return setup_priority::LATE; }

        void set_bluetooth_name(const char *name);
        void set_tcp_port(uint16_t port);
        void set_mqtt_host(const char *host);
        void set_mqtt_port(uint16_t port);
        void set_mqtt_user(const char *user);
        void set_mqtt_pass(const char *pass);
        void set_mqtt_client(const char *client);
        void set_mqtt_topic(const char *topic);
};

}  // namespace divoom
}  // namespace esphome
