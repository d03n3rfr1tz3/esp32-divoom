
#ifndef _DIVOOM_H
    #define _DIVOOM_H
    
    #include "Arduino.h"

    typedef struct {
        uint8_t data[256];
        size_t size;
    } data_command_t;

    typedef struct {
        data_command_t command[5];
        uint8_t count;
    } data_commands_t;

    class Divoom {
        public:
            data_commands_t* parseMode(char *buffer, size_t size);

        protected:
            void get_view(uint8_t value);
            void send_brightness(uint8_t value);
            void send_volume(uint8_t value);
            void send_keyboard(int8_t value);
            void send_playstate(bool value);
            void send_weather(char* value, uint8_t weather);
            void send_datetime(char* date, char* time);

            void show_clock(uint8_t clock, bool twentyfour, bool weather, bool temp, bool calendar, char* color, int8_t hot);
            void show_light(char* color, uint8_t brightness, bool power);
            void show_effects(uint8_t value);
            void show_visualization(uint8_t value);
            void show_design(int8_t value);
            void show_scoreboard(uint8_t version, uint16_t player1, uint16_t player2);
            void show_lyrics();
            void show_countdown(bool value, char* countdown);
            void show_noise(bool value);
            void show_timer(uint8_t value);
            void show_alarm(uint8_t value, char* time, bool* weekdays, uint8_t alarm, uint8_t trigger, float frequency, uint8_t volume);
            void show_memorial(uint8_t value, char* date, char* time, char* text, bool animate);
            void show_radio(bool value, float frequency);
            void show_sleep(bool value, uint8_t sleeptime, uint8_t sleepmode, float frequency, uint8_t volume, char* color, uint8_t brightness);
            void show_game(int8_t value);
            void send_gamecontrol(uint8_t value);
        
        private:
            data_commands_t commands;
            void command(data_command_t *command, uint8_t* buffer, size_t size);
            size_t checksum(uint8_t *buffer, size_t start, size_t end);
    };
#endif