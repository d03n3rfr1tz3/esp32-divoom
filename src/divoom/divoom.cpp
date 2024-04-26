
#include "divoom.h"

/**
 * Parses the MODE and returns the raw Bluetooth packet to send
*/
data_commands_t* Divoom::parseMode(char *buffer, size_t size) {

    if (size > strlen("brightness ") && strncmp("brightness ", (const char*)buffer, strlen("brightness ")) == 0) {
        size_t offset = strlen("brightness ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        send_brightness(value);
    }

    if (size > strlen("volume ") && strncmp("volume ", (const char*)buffer, strlen("volume ")) == 0) {
        size_t offset = strlen("volume ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        send_volume(value);
    }

    if (size > strlen("keyboard ") && strncmp("keyboard ", (const char*)buffer, strlen("keyboard ")) == 0) {
        size_t offset = strlen("keyboard ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        int8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtol(token, NULL, 10);

        send_keyboard(value);
    }

    if (size > strlen("playstate ") && strncmp("playstate ", (const char*)buffer, strlen("playstate ")) == 0) {
        size_t offset = strlen("playstate ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        send_playstate(value == 1);
    }

    if (size > strlen("weather ") && strncmp("weather ", (const char*)buffer, strlen("weather ")) == 0) {
        size_t offset = strlen("weather ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        char* value = 0;
        uint8_t weather = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = token;

        token = strtok(NULL, " ");
        if (token != NULL) weather = strtoul(token, NULL, 10);

        send_weather(value, weather);
    }

    return &commands;
}

/**
 * 
*/
void Divoom::command(data_command_t *command, uint8_t* modeBuffer, size_t modeSize) {
    size_t index = 0;
    command->data[index++] = 0x01; //start

    size_t commandSize = modeSize + 2;
    size_t checksumStart = index;
    size_t checksumEnd = index + commandSize;

    command->data[index++] = (commandSize & 0xff); //length
    command->data[index++] = (commandSize >> 8); //length
    
    for (size_t i = 0; i < modeSize; i++)
    {
        command->data[index++] = modeBuffer[i];
    }

    size_t modeChecksum = checksum(command->data, checksumStart, checksumEnd);
    command->data[index++] = (modeChecksum & 0xff); //checksum
    command->data[index++] = (modeChecksum >> 8); //checksum

    command->data[index++] = 0x02; //end
    command->size = index;
}

/**
 * 
*/
size_t Divoom::checksum(uint8_t *buffer, size_t start, size_t end) {
    size_t sum = 0;

    for (size_t i = start; i < end; i++) {
        sum += buffer[i];
    }

    return sum;
}

/**
 * 
*/
void Divoom::get_view(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x46;
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::send_brightness(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x74;
    buffer[index++] = value;
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::send_volume(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x08;
    buffer[index++] = (uint8_t)((uint16_t)value * 15 / 100);
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::send_keyboard(int8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x23;

    if (value == 0) {
        buffer[index++] = 0x02;
        buffer[index++] = 0x29;
    }
    if (value > 0) {
        buffer[index++] = 0x01;
        buffer[index++] = 0x28;
    }
    if (value < 0) {
        buffer[index++] = 0x00;
        buffer[index++] = 0x27;
    }
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::send_playstate(bool value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x0a;
    buffer[index++] = value ? 0x01 : 0x00;
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::send_weather(char* value, uint8_t weather) {
    if (true) {
        size_t index = 0;
        uint8_t buffer[8];

        buffer[index++] = 0x5f;
        buffer[index++] = (uint8_t)strtol(value, NULL, 10);
        buffer[index++] = weather;
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    bool isCelsius = strstr(value, "°C") != nullptr;
    bool isFahrenheit = strstr(value, "°F") != nullptr;
    if (isCelsius || isFahrenheit) {
        size_t index = 0;
        uint8_t buffer[8];

        buffer[index++] = 0x2b;

        if (isCelsius) {
            buffer[index++] = 0x00;
        }
        
        if (isFahrenheit) {
            buffer[index++] = 0x01;
        }
        
        command(&(commands.command[commands.count++]), buffer, index);
    }
}

/**
 * 
*/
void Divoom::send_datetime(char* value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x18;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/

void Divoom::show_clock(uint8_t clock, bool weather, bool temp, bool calendar, uint8_t* color, int8_t hot) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_light(uint8_t* color, uint8_t brightness, bool power) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_effects(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_visualization(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_design(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_scoreboard(uint8_t version, uint8_t player1, uint8_t player2) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_lyrics(uint8_t version) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_countdown(bool value, char* countdown) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_noise(bool value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_timer(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_alarm(uint8_t value, char* time, bool* weekdays, uint8_t alarm, uint8_t trigger, float frequency, uint8_t volume) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_memorial(uint8_t value, char* datetime, char* text, bool animate) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_radio(bool value, float frequency) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::show_game(int8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * 
*/
void Divoom::send_gamecontrol(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x00;
    //TODO
    
    command(&(commands.command[commands.count++]), buffer, index);
}