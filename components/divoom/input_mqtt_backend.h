
#ifndef _INPUT_MQTT_BACKEND_H
    #define _INPUT_MQTT_BACKEND_H

    #include "Arduino.h"

    // the mqtt transport of MqttInput, whose events go back into it
    class MqttBackend {
        public:
            static void setup(const char *clientId, const char *user, const char *pass, const char *host, uint16_t port, const char *willTopic);
            static void connect();
            static bool connected();

            static void publish(const char *topic, uint8_t qos, bool retain, const char *payload);
            static void subscribe(const char *topic, uint8_t qos);
    };
#endif
