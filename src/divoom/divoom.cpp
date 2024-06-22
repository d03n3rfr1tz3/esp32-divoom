
#include "divoom.h"

/**
 * Parses the MODE and returns the raw Bluetooth packet to send
*/
data_commands_t* Divoom::parseMode(char *buffer, size_t size) {

    if (size > strlen("brightness") && strncmp("brightness ", (const char*)buffer, strlen("brightness ")) == 0) {
        size_t offset = strlen("brightness ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        send_brightness(value);
    }

    if (size > strlen("volume") && strncmp("volume ", (const char*)buffer, strlen("volume ")) == 0) {
        size_t offset = strlen("volume ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        send_volume(value);
    }

    if (size > strlen("keyboard") && strncmp("keyboard ", (const char*)buffer, strlen("keyboard ")) == 0) {
        size_t offset = strlen("keyboard ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        int8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtol(token, NULL, 10);

        send_keyboard(value);
    }

    if (size > strlen("playstate") && strncmp("playstate ", (const char*)buffer, strlen("playstate ")) == 0) {
        size_t offset = strlen("playstate ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        send_playstate(value == 1);
    }

    if (size > strlen("weather") && strncmp("weather ", (const char*)buffer, strlen("weather ")) == 0) {
        size_t offset = strlen("weather ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        char* value = nullptr;
        uint8_t weather = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = token;

        token = strtok(NULL, " ");
        if (token != NULL) weather = strtoul(token, NULL, 10);

        send_weather(value, weather);
    }

    if (size > strlen("datetime") && strncmp("datetime ", (const char*)buffer, strlen("datetime ")) == 0) {
        size_t offset = strlen("datetime ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        char* date = nullptr;
        char* time = nullptr;

        char *token = strtok(content, " ");
        if (token != NULL) date = token;

        token = strtok(NULL, " ");
        if (token != NULL) time = token;

        send_datetime(date, time);
    }

    if (size > strlen("clock") && strncmp("clock ", (const char*)buffer, strlen("clock ")) == 0) {
        size_t offset = strlen("clock ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t clock = 0;
        bool twentyfour = 0;
        bool weather = 0;
        bool temp = 0;
        bool calendar = 0;
        char* color = nullptr;
        int8_t hot = 0;

        char *token = strtok(content, " ");
        if (token != NULL) clock = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) twentyfour = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) weather = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) temp = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) calendar = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) color = token;

        token = strtok(NULL, " ");
        if (token != NULL) hot = strtoul(token, NULL, 10);

        show_clock(clock, twentyfour, weather, temp, calendar, color, hot);
    }

    if (size > strlen("light") && strncmp("light ", (const char*)buffer, strlen("light ")) == 0) {
        size_t offset = strlen("light ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t brightness = 0;
        char* color = nullptr;
        bool power = 1;

        char *token = strtok(content, " ");
        if (token != NULL) brightness = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) color = token;

        token = strtok(NULL, " ");
        if (token != NULL) power = strtoul(token, NULL, 10);

        show_light(color, brightness, power);
    }

    if (size > strlen("effects") && strncmp("effects ", (const char*)buffer, strlen("effects ")) == 0) {
        size_t offset = strlen("effects ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        show_effects(value);
    }

    if (size > strlen("visualization") && strncmp("visualization ", (const char*)buffer, strlen("visualization ")) == 0) {
        size_t offset = strlen("visualization ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        show_visualization(value);
    }

    if (size > strlen("design") && strncmp("design ", (const char*)buffer, strlen("design ")) == 0) {
        size_t offset = strlen("design ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        int8_t value = -1;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtol(token, NULL, 10);

        show_design(value);
    }

    if (size > strlen("scoreboard") && strncmp("scoreboard ", (const char*)buffer, strlen("scoreboard ")) == 0) {
        size_t offset = strlen("scoreboard ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t version = 0;
        uint8_t player1 = 0;
        uint8_t player2 = 0;

        char *token = strtok(content, " ");
        if (token != NULL) version = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) player1 = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) player2 = strtoul(token, NULL, 10);

        show_scoreboard(version, player1, player2);
    }

    if (size >= strlen("lyrics") && strncmp("lyrics", (const char*)buffer, strlen("lyrics")) == 0) {

        show_lyrics();
    }

    if (size > strlen("countdown") && strncmp("countdown ", (const char*)buffer, strlen("countdown ")) == 0) {
        size_t offset = strlen("countdown ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        bool value = 0;
        char* countdown = nullptr;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) countdown = token;

        show_countdown(value, countdown);
    }

    if (size > strlen("noise") && strncmp("noise ", (const char*)buffer, strlen("noise ")) == 0) {
        size_t offset = strlen("noise ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        bool value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        show_noise(value);
    }

    if (size > strlen("timer") && strncmp("timer ", (const char*)buffer, strlen("timer ")) == 0) {
        size_t offset = strlen("timer ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        show_timer(value);
    }

    if (size > strlen("alarm") && strncmp("alarm ", (const char*)buffer, strlen("alarm ")) == 0) {
        size_t offset = strlen("alarm ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;
        char* time = nullptr;
        bool* weekdays = nullptr;
        uint8_t alarm = 0;
        uint8_t trigger = 0;
        float frequency = 0;
        uint8_t volume = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) time = token;

        token = strtok(NULL, " ");
        if (token != NULL)  {
            bool days[7];
            for (uint8_t i = 0; i < 7; i++) {
                days[i] = (token[i] - '0') == 1;
            }
        }

        token = strtok(NULL, " ");
        if (token != NULL) alarm = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) trigger = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) frequency = strtof(token, NULL);

        token = strtok(NULL, " ");
        if (token != NULL) volume = strtoul(token, NULL, 10);

        show_alarm(value, time, weekdays, alarm, trigger, frequency, volume);
    }

    if (size > strlen("memorial") && strncmp("memorial ", (const char*)buffer, strlen("memorial ")) == 0) {
        size_t offset = strlen("memorial ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;
        char* date = nullptr;
        char* time = nullptr;
        char text[17] = { '\0' };

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) date = token;

        token = strtok(NULL, " ");
        if (token != NULL) time = token;

        token = strtok(NULL, " ");
        if (token != NULL)  {
            bool split = false;
            for (size_t i = 0; i < 16; i++) {
                char chr = token[i];
                if (split && chr == '\0') break;
                if (!split && chr == '\0') {
                    split = true;
                    text[i] = ' ';
                } else text[i] = chr;
            }
        }

        show_memorial(value, date, time, text, true);
    }

    if (size > strlen("radio") && strncmp("radio ", (const char*)buffer, strlen("radio ")) == 0) {
        size_t offset = strlen("radio ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        bool value = 0;
        float frequency = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) frequency = strtof(token, NULL);

        show_radio(value, frequency);
    }

    if (size > strlen("sleep") && strncmp("sleep ", (const char*)buffer, strlen("sleep ")) == 0) {
        size_t offset = strlen("sleep ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        bool value = 0;
        uint8_t sleeptime = 0;
        uint8_t sleepmode = 0;
        float frequency = 0;
        uint8_t volume = 0;
        char* color = nullptr;
        uint8_t brightness = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) sleeptime = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) sleepmode = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) frequency = strtof(token, NULL);

        token = strtok(NULL, " ");
        if (token != NULL) volume = strtoul(token, NULL, 10);

        token = strtok(NULL, " ");
        if (token != NULL) color = token;

        token = strtok(NULL, " ");
        if (token != NULL) brightness = strtoul(token, NULL, 10);

        show_sleep(value, sleeptime, sleepmode, frequency, volume, color, brightness);
    }

    if (size > strlen("game") && strncmp("game ", (const char*)buffer, strlen("game ")) == 0) {
        size_t offset = strlen("game ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        int8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtol(token, NULL, 10);

        show_game(value);
    }

    if (size > strlen("gamecontrol") && strncmp("gamecontrol ", (const char*)buffer, strlen("gamecontrol ")) == 0) {
        size_t offset = strlen("gamecontrol ") * sizeof(uint8_t);
        char* content = buffer + offset;
        size -= offset;

        uint8_t value = 0;

        char *token = strtok(content, " ");
        if (token != NULL) value = strtoul(token, NULL, 10);

        send_gamecontrol(value);
    }

    return &commands;
}

/**
 * 
*/
void Divoom::command(data_command_t *command, uint8_t* modeBuffer, size_t modeSize) {
    size_t index = 0;
    command->data[index++] = 0x01; // start

    size_t commandSize = modeSize + 2;
    size_t checksumStart = index;
    size_t checksumEnd = index + commandSize;

    command->data[index++] = (commandSize & 0xff); // length
    command->data[index++] = (commandSize >> 8); // length
    
    for (size_t i = 0; i < modeSize; i++)
    {
        command->data[index++] = modeBuffer[i];
    }

    size_t modeChecksum = checksum(command->data, checksumStart, checksumEnd);
    command->data[index++] = (modeChecksum & 0xff); // checksum
    command->data[index++] = (modeChecksum >> 8); // checksum

    command->data[index++] = 0x02; // end
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
 * gets the current view info
*/
void Divoom::get_view(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x46; // get view
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * sets the brightness
*/
void Divoom::send_brightness(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x74; // set brightness
    buffer[index++] = value; // brightness
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * sets the volume
*/
void Divoom::send_volume(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x08; // set volume
    buffer[index++] = (uint8_t)((uint16_t)value * 15 / 100); // volume
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * changes the keyboard lights
*/
void Divoom::send_keyboard(int8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x23; // set keyboard lights

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
 * sets the play state
*/
void Divoom::send_playstate(bool value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x0a; // set playstate
    buffer[index++] = value ? 0x01 : 0x00; // playstate
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * sets the weather, temperature and temperature type
*/
void Divoom::send_weather(char* value, uint8_t weather) {
    if (true) {
        size_t index = 0;
        uint8_t buffer[8];

        buffer[index++] = 0x5f; // set weather
        buffer[index++] = value != nullptr ? (uint8_t)strtol(value, NULL, 10) : 0x00; // temp
        buffer[index++] = weather; // weather
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    bool isCelsius = value != nullptr && strstr(value, "°C") != nullptr;
    bool isFahrenheit = value != nullptr && strstr(value, "°F") != nullptr;
    if (isCelsius || isFahrenheit) {
        size_t index = 0;
        uint8_t buffer[4];

        buffer[index++] = 0x2b; // set temp type

        if (isCelsius) {
            buffer[index++] = 0x00; // celsius type
        }
        
        if (isFahrenheit) {
            buffer[index++] = 0x01; // fahrenheit type
        }
        
        command(&(commands.command[commands.count++]), buffer, index);
    }
}

/**
 * sets the date and time
*/
void Divoom::send_datetime(char* date, char* time) {
    size_t index = 0;
    uint8_t buffer[12];

    buffer[index++] = 0x18; // set datetime

    std::string dateString = date == nullptr ? "" : std::string(date);
    std::string timeString = time == nullptr ? "" : std::string(time);
    if (dateString.length() >= 10 && timeString.length() >= 8) {
        uint16_t year = strtoul(dateString.substr(0, 4).c_str(), NULL, 10);
        buffer[index++] = (uint8_t)(year % 100); // second year part
        buffer[index++] = (uint8_t)(year / 100); // first your part
        buffer[index++] = (uint8_t)strtoul(dateString.substr(5, 2).c_str(), NULL, 10); // month
        buffer[index++] = (uint8_t)strtoul(dateString.substr(8, 2).c_str(), NULL, 10); // day

        buffer[index++] = (uint8_t)strtoul(timeString.substr(0, 2).c_str(), NULL, 10); // hour
        buffer[index++] = (uint8_t)strtoul(timeString.substr(3, 2).c_str(), NULL, 10); // minute
        buffer[index++] = (uint8_t)strtoul(timeString.substr(6, 2).c_str(), NULL, 10); // second
    } else {
        buffer[index++] = 0x00; // second year part
        buffer[index++] = 0x00; // first your part
        buffer[index++] = 0x00; // month
        buffer[index++] = 0x00; // day

        buffer[index++] = 0x00; // hour
        buffer[index++] = 0x00; // minute
        buffer[index++] = 0x00; // second
    }
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows the clock channel
*/

void Divoom::show_clock(uint8_t clock, bool twentyfour, bool weather, bool temp, bool calendar, char* color, int8_t hot) {
    if (true) {
        size_t index = 0;
        uint8_t buffer[16];

        buffer[index++] = 0x45; // set view
        buffer[index++] = 0x00; // clock view
        buffer[index++] = twentyfour ? 0x01 : 0x00; // 12h or 24h format
        
        if (clock >= 0 && clock <= 15) {
            buffer[index++] = clock; // clock style
            buffer[index++] = 0x01; // clock on
        } else {
            buffer[index++] = 0x00; // clock style
            buffer[index++] = 0x00; // clock off
        }
        
        buffer[index++] = weather ? 0x01 : 0x00; // weather on/off
        buffer[index++] = temp ? 0x01 : 0x00; // temperature on/off
        buffer[index++] = calendar ? 0x01 : 0x00; // calendar on/off

        if (color != nullptr) { // color
            uint32_t colorLong = strtoul(color, NULL, 16);
            buffer[index++] = (colorLong >> 16 & 0xFF);
            buffer[index++] = (colorLong >> 8 & 0xFF);
            buffer[index++] = (colorLong & 0xFF);
        }
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    if (hot > 0 && hot <= 1) {
        size_t index = 0;
        uint8_t buffer[4];

        buffer[index++] = 0x26; // set hot
        buffer[index++] = hot ? 0x01 : 0x00; // hot on/off
        
        command(&(commands.command[commands.count++]), buffer, index);
    }
}

/**
 * shows the light channel
*/
void Divoom::show_light(char* color, uint8_t brightness, bool power) {
    size_t index = 0;
    uint8_t buffer[12];

    buffer[index++] = 0x45; // set view
    buffer[index++] = 0x01; // light view
    
    if (color != nullptr) { // color
        uint32_t colorLong = strtoul(color, NULL, 16);
        buffer[index++] = (colorLong >> 16 & 0xFF);
        buffer[index++] = (colorLong >> 8 & 0xFF);
        buffer[index++] = (colorLong & 0xFF);
    } else {
        buffer[index++] = 0xFF;
        buffer[index++] = 0xFF;
        buffer[index++] = 0xFF;
    }
    
    buffer[index++] = brightness; // brightness
    buffer[index++] = color == nullptr ? 0x01 : 0x00; // effect mode
    buffer[index++] = power ? 0x01 : 0x00; // power on/off
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 *  shows the effects channel
*/
void Divoom::show_effects(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x45; // set view
    buffer[index++] = 0x03; // effects view
    buffer[index++] = value; // effect
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows the visualization channel
*/
void Divoom::show_visualization(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x45; // set view
    buffer[index++] = 0x04; // visualization view
    buffer[index++] = value; // visualization
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows the design channel
*/
void Divoom::show_design(int8_t value) {
    if (true) {
        size_t index = 0;
        uint8_t buffer[4];

        buffer[index++] = 0x45; // set view
        buffer[index++] = 0x05; // design view
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    if (value >= 0 && value <= 3) {
        size_t index = 0;
        uint8_t buffer[4];

        buffer[index++] = 0xbd; // set design
        buffer[index++] = 0x17;
        buffer[index++] = (uint8_t)value; // design
        
        command(&(commands.command[commands.count++]), buffer, index);
    }
}

/**
 * shows the scoreboard channel or tool
*/
void Divoom::show_scoreboard(uint8_t version, uint16_t player1, uint16_t player2) {
    if (version == 0) {
        size_t index = 0;
        uint8_t buffer[12];

        buffer[index++] = 0x45; // set view
        buffer[index++] = 0x06; // scoreboard view
        buffer[index++] = 0x00;
        
        buffer[index++] = (player2 & 0xff); // player2
        buffer[index++] = (player2 >> 8); // player2
        
        buffer[index++] = (player1 & 0xff); // player1
        buffer[index++] = (player1 >> 8); // player1
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    if (version == 1) {
        size_t index = 0;
        uint8_t buffer[12];

        buffer[index++] = 0x72; // set tool
        buffer[index++] = 0x01; // scoreboard tool
        buffer[index++] = 0x01; // show it
        
        buffer[index++] = (player2 & 0xff); // player2
        buffer[index++] = (player2 >> 8); // player2
        
        buffer[index++] = (player1 & 0xff); // player1
        buffer[index++] = (player1 >> 8); // player1
        
        command(&(commands.command[commands.count++]), buffer, index);
    }
}

/**
 * shows the lyrics tool
*/
void Divoom::show_lyrics() {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x45; // set view
    buffer[index++] = 0x06; // lyrics view - on older devices this is the scoreboard view

    buffer[index++] = 0x00;
    buffer[index++] = 0x00;
    buffer[index++] = 0x00;
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows a countdown
*/
void Divoom::show_countdown(bool value, char* countdown) {
    size_t index = 0;
    uint8_t buffer[8];

    buffer[index++] = 0x72; // set tool
    buffer[index++] = 0x03; // countdown tool
    buffer[index++] = value ? 0x01 : 0x00; // on/off
    
    std::string timeString = countdown == nullptr ? "" : std::string(countdown);
    if (timeString.length() >= 5) {
        buffer[index++] = (uint8_t)strtoul(timeString.substr(0, 2).c_str(), NULL, 10); // hour
        buffer[index++] = (uint8_t)strtoul(timeString.substr(3, 2).c_str(), NULL, 10); // minute
    } else {
        buffer[index++] = 0x00; // hour
        buffer[index++] = 0x00; // minute
    }

    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows the noise meter
*/
void Divoom::show_noise(bool value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x72; // set tool
    buffer[index++] = 0x02; // noise tool
    buffer[index++] = value ? 0x01 : 0x02; // on/off
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows a timer
*/
void Divoom::show_timer(uint8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x72; // set tool
    buffer[index++] = 0x00; // timer tool
    buffer[index++] = value; // on/off/reset
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * sets an alarm
*/
void Divoom::show_alarm(uint8_t value, char* time, bool* weekdays, uint8_t alarm, uint8_t trigger, float frequency, uint8_t volume) {
    size_t index = 0;
    uint8_t buffer[16];

    buffer[index++] = 0x43; // set alarm
    buffer[index++] = value; // alarm slot

    std::string timeString = time == nullptr ? "" : std::string(time);
    if (timeString.length() >= 5) {
        buffer[index++] = 0x01; // alarm on
        buffer[index++] = (uint8_t)strtoul(timeString.substr(0, 2).c_str(), NULL, 10); // hour
        buffer[index++] = (uint8_t)strtoul(timeString.substr(3, 2).c_str(), NULL, 10); // minute
    } else {
        buffer[index++] = 0x00; // alarm off
        buffer[index++] = 0x00; // hour
        buffer[index++] = 0x00; // minute
    }

    if (weekdays != nullptr) {
        uint8_t weekbits = 0;
        if (weekdays[0]) weekbits += 1;
        if (weekdays[1]) weekbits += 2;
        if (weekdays[2]) weekbits += 4;
        if (weekdays[3]) weekbits += 8;
        if (weekdays[4]) weekbits += 16;
        if (weekdays[5]) weekbits += 32;
        if (weekdays[6]) weekbits += 64;
        buffer[index++] = weekbits; // weekday bits
    } else {
        buffer[index++] = 0x00; // weekday bits
    }

    buffer[index++] = alarm; // alarm mode
    buffer[index++] = trigger; // trigger mode

    if (frequency > 0) { // frequency
        uint16_t freq = frequency * 10;
        if (freq > 1000) {
            buffer[index++] = (uint8_t)(freq - 1000);
            buffer[index++] = (uint8_t)(freq / 100);
        } else {
            buffer[index++] = (uint8_t)(freq % 100);
            buffer[index++] = (uint8_t)(freq / 100);
        }
    } else {
        buffer[index++] = 0x00;
        buffer[index++] = 0x00;
    }
    
    buffer[index++] = volume; // volume
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * sets a memorial (reminder)
*/
void Divoom::show_memorial(uint8_t value, char* date, char* time, char* text, bool animate) {
    size_t index = 0;
    uint8_t buffer[48];

    buffer[index++] = 0x54; // set memorial
    buffer[index++] = value; // memorial slot
    
    std::string dateString = date == nullptr ? "" : std::string(date);
    std::string timeString = time == nullptr ? "" : std::string(time);
    if (dateString.length() >= 10 && timeString.length() >= 8) {
        buffer[index++] = 0x01; // alarm on
        buffer[index++] = (uint8_t)strtoul(dateString.substr(5, 2).c_str(), NULL, 10); // month
        buffer[index++] = (uint8_t)strtoul(dateString.substr(8, 2).c_str(), NULL, 10); // day

        buffer[index++] = (uint8_t)strtoul(timeString.substr(0, 2).c_str(), NULL, 10); // hour
        buffer[index++] = (uint8_t)strtoul(timeString.substr(3, 2).c_str(), NULL, 10); // minute
    } else {
        buffer[index++] = 0x00; // alarm off
        buffer[index++] = 0x00; // month
        buffer[index++] = 0x00; // day

        buffer[index++] = 0x00; // hour
        buffer[index++] = 0x00; // minute
    }
    
    buffer[index++] = animate ? 0x01 : 0x00; // animate on/off

    bool end = false;
    if (text == nullptr) text = (char*)"ESP32\0";
    for (size_t i = 0; i < 16; i++) // name of memorial
    {
        if (text[i] == '\0') end = true;
        uint16_t chr = end ? '\0' : text[i];
        buffer[index++] = (chr >> 8);
        buffer[index++] = (chr & 0xff);
    }

    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows the radio
*/
void Divoom::show_radio(bool value, float frequency) {
    if (true) {
        size_t index = 0;
        uint8_t buffer[4];

        buffer[index++] = 0x05; // set radio
        buffer[index++] = value ? 0x01 : 0x00; // on/off
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    if (value && frequency > 0) {
        size_t index = 0;
        uint8_t buffer[8];

        buffer[index++] = 0x61; // set radio frequency
        
        uint16_t freq = frequency * 10;
        if (freq > 1000) { // frequency
            buffer[index++] = (uint8_t)(freq - 1000);
            buffer[index++] = (uint8_t)(freq / 100);
        } else {
            buffer[index++] = (uint8_t)(freq % 100);
            buffer[index++] = (uint8_t)(freq / 100);
        }
        
        command(&(commands.command[commands.count++]), buffer, index);
    }
}

/**
 * shows the sleep mode
*/
void Divoom::show_sleep(bool value, uint8_t sleeptime, uint8_t sleepmode, float frequency, uint8_t volume, char* color, uint8_t brightness) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0x40; // set radio
    buffer[index++] = sleeptime; // sleep time in minutes
    buffer[index++] = sleepmode; // sleep mode
    buffer[index++] = value ? 0x01 : 0x00; // on/off

    uint16_t freq = frequency * 10;
    if (freq > 1000) { // frequency
        buffer[index++] = (uint8_t)(freq - 1000);
        buffer[index++] = (uint8_t)(freq / 100);
    } else {
        buffer[index++] = (uint8_t)(freq % 100);
        buffer[index++] = (uint8_t)(freq / 100);
    }
    
    buffer[index++] = volume; // volume
    
    if (color != nullptr) { // color
        uint32_t colorLong = strtoul(color, NULL, 16);
        buffer[index++] = (colorLong >> 16 & 0xFF);
        buffer[index++] = (colorLong >> 8 & 0xFF);
        buffer[index++] = (colorLong & 0xFF);
    } else {
        buffer[index++] = 0x00;
        buffer[index++] = 0x00;
        buffer[index++] = 0x00;
    }
    
    buffer[index++] = brightness; // brightness

    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * shows a game
*/
void Divoom::show_game(int8_t value) {
    size_t index = 0;
    uint8_t buffer[4];

    buffer[index++] = 0xa0;
    buffer[index++] = value >= 0 ? 0x01 : 0x00;
    buffer[index++] = value >= 0 ? (uint8_t)value : 0;
    
    command(&(commands.command[commands.count++]), buffer, index);
}

/**
 * sends controls for a game
*/
void Divoom::send_gamecontrol(uint8_t value) {
    if (value == 0) {
        size_t index = 0;
        uint8_t buffer[2];

        buffer[index++] = 0x88;
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    if (value > 0) {
        size_t index = 0;
        uint8_t buffer[4];

        buffer[index++] = 0x17;
        buffer[index++] = value;
        
        command(&(commands.command[commands.count++]), buffer, index);
    }

    if (value > 0) {
        size_t index = 0;
        uint8_t buffer[4];

        buffer[index++] = 0x21;
        buffer[index++] = value;
        
        command(&(commands.command[commands.count++]), buffer, index);
    }
}