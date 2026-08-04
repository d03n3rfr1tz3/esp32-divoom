
#ifndef _INPUT_MQTT_H
    #define _INPUT_MQTT_H

    #include "Arduino.h"
    #include "input_mqtt_backend.h"

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
            static void advertise(const uint8_t* address, const char* name, size_t size, bool supported);

            // invoked by MqttBackend
            static void connected(bool sessionPresent);
            static void disconnected();
            static void message(const char* topic, const char* payload, size_t len, size_t index, size_t total);

        private:
            inline static bool isConnected;
            inline static bool wasWifiConnected;
            inline static unsigned long timer;
            inline static char topicState[64];
            inline static char topicHeap[64];
            inline static char topicBluetooth[64];
            inline static char topicCommand[64];
            inline static char messageBuffer[1024];

            static void parse(const char* topic, char* payload, size_t size);
    };
#endif