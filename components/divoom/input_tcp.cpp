
#include "input_tcp.h"

#include "platform.h"
#include "settings.h"
#include "input_base.h"
#include "output_base.h"

// constructed in setup, so the port can come from the settings
AsyncServer* tcpServer = nullptr;
AsyncClient* tcpClients[TCP_MAX];

static frame_stream_t frameStream;

/**
 * setup functionality
*/
void TcpInput::setup() {
    tcpServer = new AsyncServer(SettingsHandler::tcpPort);
    tcpServer->onClient(connection, tcpServer);
    tcpServer->begin();

    parsePacketQueue = xQueueCreate(3, sizeof(data_packet_t*));
    
    BaseType_t taskResult = xTaskCreatePinnedToCore(queue, "ParsePacketTask", 3072, NULL, 1, &parsePacketHandle, 1);
    if (taskResult != pdPASS) DIVOOM_FAIL("could not create the parse packet task");
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

    int8_t index = -1;
    for (size_t i = 0; i < TCP_MAX; i++)
    {
        if (tcpClients[i] != nullptr) continue;
        index = i;
        break;
    }

    if (index < 0) index = TcpInput::clear();
    tcpClients[index] = client;
}

/**
 * clear the oldest known client connection and return its slot
*/
int8_t TcpInput::clear() {
	for (size_t i = 0; i < TCP_MAX; i++)
    {
        if (tcpClients[i] == nullptr) continue;

        AsyncClient *client = tcpClients[i];
        tcpClients[i] = nullptr;
        client->close();
        delete client;
        return i;
    }
    return 0;
}

/**
 * callback for when a client send data
*/
void TcpInput::data(void *arg, AsyncClient *client, void *data, size_t size) {
    data_packet_t* dataPacket = (data_packet_t*)MALLOC(sizeof(data_packet_t));
    if (!dataPacket) DIVOOM_FAIL("out of memory for an incoming packet");

    if (size > sizeof(dataPacket->data)) size = sizeof(dataPacket->data);
    dataPacket->size = size;
    memcpy(dataPacket->data, (uint8_t*)data, size);

    if (xQueueSend(parsePacketQueue, (void*)&dataPacket, (TickType_t)25) == errQUEUE_FULL) {
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
        tcpClients[i] = nullptr;
        client->close();
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
        tcpClients[i] = nullptr;
        client->abort();
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
    DIVOOM_WDT_ADD();

    for (;;) {
        data_packet_t* dataPacket;
        if (xQueueReceive(parsePacketQueue, &dataPacket, (TickType_t)25) == pdPASS) {
            // splitting along the frame structure sends every animation frame as a separate packet
            appendFrames(&frameStream, dataPacket->data, dataPacket->size, TcpInput::parse);
            free(dataPacket);
        }

        DIVOOM_WDT_RESET();
        vTaskDelay(1);
    }
}

/**
 * the parser for incoming data
*/
void TcpInput::parse(const uint8_t *buffer, size_t size) {
    if (size == 0) return;

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