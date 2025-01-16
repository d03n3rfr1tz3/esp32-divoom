
#ifndef _INPUT_TCP_H
    #define _INPUT_TCP_H

    #include "config.h"
    #include "util.h"
    
    #include "Arduino.h"
    #include "AsyncTCP.h"

    class TcpInput {
        public:
            static void setup();
            static void loop();
            
            static void forward(const char *address, uint16_t port);
            static void forward(const uint8_t *buffer, size_t size);
            static void backward(const uint8_t *buffer, size_t size);
            static void advertise(const uint8_t* address, const char* name, size_t size);

            static void connection(void *arg, AsyncClient *client);
            static void data(void *arg, AsyncClient *client, void *data, size_t size);
            static void timeout(void *arg, AsyncClient *client, uint32_t time);
            static void error(void *arg, AsyncClient *client, int8_t error);
            static void disconnect(void *arg, AsyncClient *client);
        
        private:
            inline static QueueHandle_t parsePacketQueue = NULL;
            inline static TaskHandle_t parsePacketHandle = NULL;
            
            static void queue(void *parameter);
            static void parse(const uint8_t *buffer, size_t size);
            static void write(const uint8_t *buffer, size_t size);
    };
#endif