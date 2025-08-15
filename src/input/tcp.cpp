
#include "tcp.h"

#include "input/base.h"
#include "output/base.h"

AsyncServer tcpServer(TCP_PORT);
AsyncClient* tcpClients[TCP_MAX];

/**
 * setup functionality
*/
void TcpInput::setup() {
    tcpServer.onClient(connection, &tcpServer);
    tcpServer.begin();

    parsePacketQueue = xQueueCreate(5, sizeof(data_packet_t*));
    xTaskCreatePinnedToCore(queue, "ParsePacketTask", 4096, NULL, 1, &parsePacketHandle, 1);
    esp_task_wdt_add(parsePacketHandle);
}

/**
 * loop functionality
*/
void TcpInput::loop() {
    // no additional handling needed
}

/**
 * the forward channel for a bluetooth connection
*/
void TcpInput::forward(const char *address, uint16_t port) {
    // no additional handling needed
}

/**
 * the forward channel for bluetooth data
*/
void TcpInput::forward(const uint8_t *buffer, size_t size) {
    // no additional handling needed
}

/**
 * the backward channel for bluetooth data
*/
void TcpInput::backward(const uint8_t *buffer, size_t size) {
    TcpInput::write(buffer, size);
}

/**
 * the channel for an advertised bluetooth device
*/
void TcpInput::advertise(const uint8_t* address, const char* name, size_t size, bool supported) {
    if (!supported) return;
    
    size_t index = 0;
    size_t length = ESP_BD_ADDR_LEN + size + 2;
    uint8_t buffer[length];

    buffer[index++] = 0x00;
    for (size_t i = 0; i < ESP_BD_ADDR_LEN; i++)
    {
        buffer[index++] = (uint8_t)address[i];
    }
    
    buffer[index++] = size;
    for (size_t i = 0; i < size; i++)
    {
        buffer[index++] = (uint8_t)name[i];
    }

    TcpInput::write(buffer, length);
}

/**
 * callback for when a client connected to our TCP port
*/
void TcpInput::connection(void *arg, AsyncClient *client) {
	client->onData(&data, NULL);
	client->onDisconnect(&disconnect, NULL);
	client->onTimeout(&timeout, NULL);
    client->onError(&error, NULL);

    for (size_t i = 0; i < TCP_MAX; i++)
    {
        if (tcpClients[i] != nullptr) continue;
        tcpClients[i] = client;
        break;
    }
}

/**
 * callback for when a client send data
*/
void TcpInput::data(void *arg, AsyncClient *client, void *data, size_t size) {
    data_packet_t* dataPacket = (data_packet_t*)MALLOC(sizeof(data_packet_t));
    dataPacket->size = size;
    memcpy(dataPacket->data, (uint8_t*)data, size);

    if (xQueueSend(parsePacketQueue, (void*)&dataPacket, (TickType_t)10) == errQUEUE_FULL) {
        free(dataPacket);
    }
}

/**
 * callback for when a client timed out
*/
void TcpInput::timeout(void *arg, AsyncClient *client, uint32_t time) {
	for (size_t i = 0; i < TCP_MAX; i++)
    {
        if (tcpClients[i] != client) continue;
        tcpClients[i]->abort();
        tcpClients[i] = nullptr;
        delete client;
        break;
    }
}

/**
 * callback for when a client error happens
*/
void TcpInput::error(void *arg, AsyncClient *client, int8_t error) {
	for (size_t i = 0; i < TCP_MAX; i++)
    {
        if (tcpClients[i] != client) continue;
        tcpClients[i]->close(true);
        tcpClients[i] = nullptr;
        delete client;
        break;
    }
}

/**
 * callback for when a client disconnected
*/
void TcpInput::disconnect(void *arg, AsyncClient *client) {
	for (size_t i = 0; i < TCP_MAX; i++)
    {
        if (tcpClients[i] != client) continue;
        tcpClients[i]->close();
        tcpClients[i] = nullptr;
        delete client;
        break;
    }
}

/**
 * the queue handler
*/
void TcpInput::queue(void *parameter) {
    size_t maxDefault = 1096;
    size_t maxAnimations[] = {210, 213, 215};

    size_t previousSize = 0;
    uint8_t previousBuffer[maxDefault] = { 0x00 };

    while (true) {
        data_packet_t* dataPacket;
        if (xQueueReceive(parsePacketQueue, &dataPacket, (TickType_t)10) == pdPASS) {
            size_t off = 0;
            size_t max = maxDefault;
            size_t len = previousSize + dataPacket->size;
            uint8_t *packetBuffer = dataPacket->data;

            // prepare packet split size, if necessary
            for (size_t maxAnimation : maxAnimations) {
                uint8_t posAnimation = maxAnimation - 1;
                uint8_t *startBuffer = previousSize > 0 ? previousBuffer : packetBuffer;
                uint8_t *endBuffer = previousSize > maxAnimation ? previousBuffer : packetBuffer;
                uint8_t endPosition = previousSize > maxAnimation ? posAnimation : posAnimation - previousSize;
                if (startBuffer[0] == 0x01 && startBuffer[3] == 0x49 && endBuffer[endPosition] == 0x02) max = maxAnimation; // animation stream detected. splitting accordingly
            }
            
            // split buffer into corresponding max size packets, to make sure animation stream sends every frame as a separate packet
            while (len > 0) {
                size_t thisSize = 0;
                uint8_t thisBuffer[max] = { 0x00 };

                // prepare packet
                size_t use = len > max ? max : len;
                if (previousSize > 0) memcpy(thisBuffer, previousBuffer, previousSize);
                memcpy(thisBuffer + previousSize, packetBuffer, use - previousSize);

                // parse and process packet
                TcpInput::parse(thisBuffer, use);
                
                // move pointer and reduce open length
                off += use - previousSize;
                packetBuffer += use - previousSize;
                len = dataPacket->size - off;
                previousSize = 0;

                // copy rest into separate buffer instead of sending incomplete data, if there is another message in the queue
                if (len <= max && packetBuffer[len - 1] != 0x02) {
                    previousSize = len;
                    memcpy(previousBuffer, packetBuffer, len);
                    break;
                }
            }

            free(dataPacket);
        }

        esp_task_wdt_reset();
        vTaskDelay(5);
    }
}

/**
 * the parser for incoming data
*/
void TcpInput::parse(const uint8_t *buffer, size_t size) {

    // recognize a connect statement and pass it into Bluetooth handler
    if (buffer[0] == 0x69 && size >= ESP_BD_ADDR_LEN + 1 && size <= ESP_BD_ADDR_LEN + 2) {
        esp_bd_addr_t bytes;
        uint16_t port = 1;

        for (size_t i = 0; i < ESP_BD_ADDR_LEN; i++)
        {
            bytes[i] = buffer[i + 1];
        }

        if (size > ESP_BD_ADDR_LEN + 1) {
            port = buffer[7];
        }
        
        BTAddress address(bytes);
        BaseInput::forward(address.toString().c_str(), port);
        BluetoothOutput::setup(address, port);
    }

    // recognize a disconnect statement and pass it into Bluetooth handler
    if (buffer[0] == 0x96 && size == ESP_BD_ADDR_LEN + 1) {
        esp_bd_addr_t bytes;
        uint16_t port = 0;

        for (size_t i = 0; i < ESP_BD_ADDR_LEN; i++)
        {
            bytes[i] = buffer[i + 1];
        }

        BTAddress address(bytes);
        BaseInput::forward(address.toString().c_str(), port);
        BluetoothOutput::setup(address, port);
    }

    // recognize a raw statement and pass it into Output handlers
    if (buffer[0] == 0x01 && buffer[size - 1] == 0x02) {
        BaseInput::forward(buffer, size);
        BaseOutput::forward(buffer, size);
    }
}

/**
 * helper for sending data back to the clients
*/
void TcpInput::write(const uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < TCP_MAX; i++)
    {
        AsyncClient* client = tcpClients[i];
        if (client == nullptr) continue;
        if (client->connected() == false) continue;

        if (client->space() > size && client->canSend())
        {
            client->add((const char *)buffer, size);
            client->send();
        }
    }
}