
#include "serial.h"

#include "input/base.h"
#include "output/base.h"

HardwareSerial serialIn(0);

/**
 * setup functionality
*/
void SerialInput::setup() {
    serialIn.begin(9600);
}

/**
 * loop functionality
*/
void SerialInput::loop() {
    uint8_t buffer[64];
    while (serialIn.available()) {
        size_t size = serialIn.readBytesUntil('\n', buffer, 64);
        if (buffer[size - 1] == '\r') size -= sizeof(char);
        buffer[size] = '\0';

        parse((const uint8_t *)buffer, size);
    }
}

/**
 * the forward channel for a bluetooth connection
*/
void SerialInput::forward(const char *address, uint16_t port) {
    serialIn.print("CONNECT ");
    serialIn.print(address);
    serialIn.print(" ");
    serialIn.println(port);
}

/**
 * the forward channel for bluetooth data
*/
void SerialInput::forward(const uint8_t *buffer, size_t size) {
    SerialInput::write("SEND", buffer, size);
}

/**
 * the backward channel for bluetooth data
*/
void SerialInput::backward(const uint8_t *buffer, size_t size) {
    SerialInput::write("RECV", buffer, size);
}

/**
 * the channel for an advertised bluetooth device
*/
void SerialInput::advertise(const uint8_t* address, const char* name, size_t size) {
    serialIn.print("ADVERTISE ");
    for (size_t i = 0; i < ESP_BD_ADDR_LEN; i++)
    {
        uint8_t byte = address[i];
        if (i > 0) serialIn.print(":");
        if (byte < 16) serialIn.print("0");
        serialIn.print(byte, HEX);
    }
    serialIn.print(" ");
    serialIn.println(name);
}

/**
 * the parser for incoming data
*/
void SerialInput::parse(const uint8_t *buffer, size_t size) {
    char* input = (char*)buffer;

    // recognize a connect statement and pass it into Bluetooth handler
    if (strncmp("CONNECT ", input, strlen("CONNECT ")) == 0) {
        size_t offset = strlen("CONNECT ") * sizeof(uint8_t);
        char* content = input + offset;
        size -= offset;

        size_t index = 0;
        esp_bd_addr_t bytes;
        uint16_t port = 1;

        char *token = strtok(content, ":");
        while (token != NULL) {
            bytes[index++] = strtoul(token, NULL, 16);
            token = strtok(NULL, ":");
        }

        if (size > 18) {
            port = content[18] - '0';
        }

        BTAddress address(bytes);
        BaseInput::forward(address.toString().c_str(), port);
        BluetoothOutput::setup(address, port);
    }

    // recognize a mode statement and pass it into Divoom handler
    if (strncmp("MODE ", input, strlen("MODE ")) == 0) {
        size_t offset = strlen("MODE ") * sizeof(uint8_t);
        char* content = input + offset;
        size -= offset;

        //TODO: Pass the parsed content into divoom.h functions
    }

    // recognize a raw statement and pass it into Output handlers
    if (strncmp("SEND ", input, strlen("SEND ")) == 0) {
        size_t offset = strlen("SEND ") * sizeof(uint8_t);
        char* content = input + offset;
        size -= offset;

        size_t index = 0;
        uint8_t bytes[size / 2];

        char *token = strtok(content, " ");
        while (token != NULL) {
            bytes[index++] = strtoul(token, NULL, 16);
            token = strtok(NULL, " ");
        }

        BaseInput::forward(bytes, index);
        BaseOutput::forward(bytes, index);
    }
}

/**
 * helper for sending data to serial
*/
void SerialInput::write(const char* direction, const uint8_t *buffer, size_t size) {
    serialIn.print(direction);
    serialIn.print(" ");

    for (size_t i = 0; i < size; i++)
    {
        uint8_t byte = buffer[i];
        if (i > 0) serialIn.print(" ");
        if (byte < 16) serialIn.print("0");
        serialIn.print(byte, HEX);
    }
    serialIn.println("");
}
