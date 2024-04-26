
#include "serial.h"

#include "input/base.h"
#include "output/base.h"

#include "divoom/divoom.h"

HardwareSerial serialIn(0);

/**
 * setup functionality
*/
void SerialInput::setup() {
    serialIn.begin(115200);
}

/**
 * loop functionality
*/
void SerialInput::loop() {
    char buffer[256];
    while (serialIn.available()) {
        size_t size = serialIn.readBytesUntil('\n', buffer, 256);
        if (buffer[size - 1] == '\r') size -= sizeof(char);
        buffer[size] = '\0';

        SerialInput::parse((char*)buffer, size);
    }
}

/**
 * the forward channel for a bluetooth connection
*/
void SerialInput::forward(const char *address, uint16_t port) {
    if (port > 0) {
        serialIn.print("CONNECT ");
        serialIn.print(address);
        serialIn.print(" ");
        serialIn.println(port);
    } else {
        serialIn.print("DISCONNECT ");
        serialIn.println(address);
    }
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
void SerialInput::parse(char *buffer, size_t size) {
    
    // recognize a connect statement and pass it into Bluetooth handler
    if (size > strlen("CONNECT ") && strncmp("CONNECT ", (const char*)buffer, strlen("CONNECT ")) == 0) {
        size_t offset = strlen("CONNECT ") * sizeof(uint8_t);
        char* content = buffer + offset;
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

    // recognize a disconnect statement and pass it into Bluetooth handler
    if (size > strlen("DISCONNECT ") && strncmp("DISCONNECT ", (const char*)buffer, strlen("DISCONNECT ")) == 0) {
        size_t offset = strlen("DISCONNECT ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        size_t index = 0;
        esp_bd_addr_t bytes;
        uint16_t port = 0;

        char *token = strtok(content, ":");
        while (token != NULL) {
            bytes[index++] = strtoul(token, NULL, 16);
            token = strtok(NULL, ":");
        }

        BTAddress address(bytes);
        BaseInput::forward(address.toString().c_str(), port);
        BluetoothOutput::setup(address, port);
    }

    // recognize a mode statement and pass it into Divoom handler
    if (size > strlen("MODE ") && strncmp("MODE ", (const char*)buffer, strlen("MODE ")) == 0) {
        size_t offset = strlen("MODE ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        Divoom divoom = Divoom();
        data_commands_t *commands = divoom.parseMode(content, size);

        for (size_t i = 0; i < commands->count; i++)
        {
            data_command_t command = commands->command[i];
            BaseInput::forward(command.data, command.size);
            BaseOutput::forward(command.data, command.size);
        }
    }

    // recognize a raw statement and pass it into Output handlers
    if (size > strlen("SEND ") && strncmp("SEND ", (const char*)buffer, strlen("SEND ")) == 0) {
        size_t offset = strlen("SEND ") * sizeof(uint8_t);
        char* content = buffer + offset;
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
