
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
void TcpInput::advertise(const uint8_t* address, const char* name, size_t size) {
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
    parse((const uint8_t *)data, size);
}

/**
 * callback for when a client timed out
*/
void TcpInput::timeout(void *arg, AsyncClient *client, uint32_t time) {
	for (size_t i = 0; i < TCP_MAX; i++)
    {
        if (tcpClients[i] != client) continue;
        tcpClients[i]->close();
        tcpClients[i] = nullptr;
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
        break;
    }
}

/**
 * the parser for incoming data
*/
void TcpInput::parse(const uint8_t *data, size_t size) {

    // recognize a connect statement and pass it into Bluetooth handler
    if (data[0] == 0x00 && size > ESP_BD_ADDR_LEN) {
        esp_bd_addr_t bytes;
        uint16_t port = 1;

        for (size_t i = 0; i < ESP_BD_ADDR_LEN; i++)
        {
            bytes[i] = (uint8_t)data[i + 1];
        }

        if (size > ESP_BD_ADDR_LEN + 1) {
            port = data[7];
        }
        
        BTAddress address(bytes);
        BaseInput::forward(address.toString().c_str(), port);
        BluetoothOutput::setup(address, port);
    }

    // recognize a raw statement and pass it into Output handlers
    if (data[0] == 0x01) {
        BaseInput::forward(data, size);
        BaseOutput::forward(data, size);
    }
}

/**
 * helper for sending data back to the clients
*/
void TcpInput::write(const uint8_t *data, size_t size) {
    for (size_t i = 0; i < TCP_MAX; i++)
    {
        AsyncClient* client = tcpClients[i];
        if (client == nullptr) continue;

        if (client->space() > size && client->canSend())
        {
            client->add((const char *)data, size);
            client->send();
        }
    }
}