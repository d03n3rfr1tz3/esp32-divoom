
#ifndef _INPUT_MQTT_H
    #define _INPUT_MQTT_H

    #include "Arduino.h"
    #include "PubSubClient.h"
    #include "WiFiClient.h"

    class MqttInput {
        public:
            MqttInput();
            static void setup();
            static void loop();
            static bool check();
            static void update();
            
            static void forward(const char *address, uint16_t port);
            static void forward(const uint8_t *buffer, size_t size);
            static void backward(const uint8_t *buffer, size_t size);
            static void advertise(const uint8_t* address, const char* name, size_t size);
        
        private:
            inline static bool isConnected;
            inline static bool wasWifiConnected;
            inline static unsigned long timer;

            static void parse(char* topic, uint8_t* payload, size_t size);
    };
#endif