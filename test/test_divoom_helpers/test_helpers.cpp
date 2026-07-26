#include <unity.h>

#include <stdio.h>
#include <string.h>

#include "cases.h"
#include "divoom/divoom.h"

void setUp(void) {}
void tearDown(void) {}

static Divoom divoom;

/**
 * parseMode mutates its buffer via strtok, so it gets a writable copy, and every
 * call gets a fresh value-initialized instance because parseMode never resets
 * commands.count and Divoom has no constructor.
*/
static data_commands_t* encode(const char* command) {
    static char buffer[256];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "%s", command);

    divoom = Divoom();
    return divoom.parseMode(buffer, strlen(buffer));
}

static uint8_t count_of(const char* command) {
    return encode(command)->count;
}

/** payload byte at `offset`, counting from the first byte after the length field */
static uint8_t payload_at(const char* command, uint8_t frame, size_t offset) {
    return encode(command)->command[frame].data[3 + offset];
}

/**
 * Every frame is `01 | len_lo | len_hi | payload | ck_lo | ck_hi | 02` with
 * len = payload + 2 and the checksum summing the length bytes and the payload
 * (see Divoom::command and Divoom::checksum). Verified against every registered
 * case rather than a single one.
*/
static void test_framing_invariant_over_all_cases(void) {
    for (size_t i = 0; i < CASE_COUNT; i++) {
        data_commands_t* commands = encode(CASES[i].command);

        for (uint8_t c = 0; c < commands->count; c++) {
            const uint8_t* data = commands->command[c].data;
            size_t size = commands->command[c].size;

            TEST_ASSERT_TRUE(size >= 6);
            TEST_ASSERT_EQUAL_HEX8(0x01, data[0]);
            TEST_ASSERT_EQUAL_HEX8(0x02, data[size - 1]);

            uint16_t length = data[1] | (data[2] << 8);
            TEST_ASSERT_EQUAL_UINT16(size - 4, length);

            uint16_t sum = 0;
            for (size_t b = 1; b <= size - 4; b++) sum += data[b];
            TEST_ASSERT_EQUAL_HEX16(sum, data[size - 3] | (data[size - 2] << 8));
        }
    }
}

/** the only case whose checksum exceeds 8 bits; catches a swapped byte order */
static void test_checksum_high_byte_is_little_endian(void) {
    data_commands_t* commands = encode("light 77 ff0080 1");
    const data_command_t* frame = &commands->command[0];

    TEST_ASSERT_EQUAL_HEX8(0x20, frame->data[frame->size - 3]);
    TEST_ASSERT_EQUAL_HEX8(0x02, frame->data[frame->size - 2]);
}

/** send_volume maps 0..100 onto 0..15 by integer division */
static void test_volume_is_scaled_to_fifteen_steps(void) {
    TEST_ASSERT_EQUAL_HEX8(0x09, payload_at("volume 60", 0, 1));
    TEST_ASSERT_EQUAL_HEX8(0x0f, payload_at("volume 100", 0, 1));
    TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("volume 6", 0, 1));
    TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("volume 0", 0, 1));
}

/** builders that emit a second frame only for some arguments */
static void test_frame_counts_depend_on_arguments(void) {
    TEST_ASSERT_EQUAL_UINT8(2, count_of("weather 22°C 3"));
    TEST_ASSERT_EQUAL_UINT8(1, count_of("weather 22 3"));

    TEST_ASSERT_EQUAL_UINT8(2, count_of("radio 1 101.3"));
    TEST_ASSERT_EQUAL_UINT8(1, count_of("radio 0 0"));

    TEST_ASSERT_EQUAL_UINT8(2, count_of("gamecontrol 1"));
    TEST_ASSERT_EQUAL_UINT8(1, count_of("gamecontrol 0"));

    TEST_ASSERT_EQUAL_UINT8(2, count_of("design 2"));
    TEST_ASSERT_EQUAL_UINT8(1, count_of("design 9"));

    TEST_ASSERT_EQUAL_UINT8(1, count_of("clock 3"));
    TEST_ASSERT_EQUAL_UINT8(2, count_of("clock 3 1"));
    TEST_ASSERT_EQUAL_UINT8(3, count_of("clock 3 1 1 1 1 0a141e 1"));
}

/** every prefix is matched with `size > strlen(prefix)`, so the bare word cannot match */
static void test_prefix_without_argument_emits_nothing(void) {
    TEST_ASSERT_EQUAL_UINT8(0, count_of("brightness"));
    TEST_ASSERT_EQUAL_UINT8(0, count_of("volume"));
    TEST_ASSERT_EQUAL_UINT8(0, count_of("alarm"));
    TEST_ASSERT_EQUAL_UINT8(0, count_of(""));
}

/** send_datetime needs a 10 character date and an 8 character time, else it zeroes both */
static void test_datetime_falls_back_to_zeroes_when_time_is_short(void) {
    for (size_t offset = 1; offset <= 7; offset++) {
        TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("datetime 2024-01-02 03:04", 0, offset));
    }
}

/** show_alarm needs a 5 character time, else it reports the alarm as off */
static void test_alarm_falls_back_to_off_when_time_is_short(void) {
    TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("alarm 1 7:3 1010100 1 1 101.1 80", 0, 2));
    TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("alarm 1 7:3 1010100 1 1 101.1 80", 0, 3));
    TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("alarm 1 7:3 1010100 1 1 101.1 80", 0, 4));
}

/**
 * Known defect, frozen deliberately: parseMode fills a block-local `bool days[7]`
 * and never assigns it to `weekdays`, which stays nullptr, so show_alarm always
 * writes empty weekday bits. hass-divoom encodes mon/wed/fri as 0x2a here.
*/
static void test_alarm_weekdays_are_ignored(void) {
    TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("alarm 1 07:30 1010100 1 1 101.1 80", 0, 5));
    TEST_ASSERT_EQUAL_HEX8(0x00, payload_at("alarm 1 07:30 1111111 1 1 101.1 80", 0, 5));
}

/** neither the view nor the tool branch of show_scoreboard accepts version 2 */
static void test_scoreboard_with_unknown_version_emits_nothing(void) {
    TEST_ASSERT_EQUAL_UINT8(0, count_of("scoreboard 2 1 2"));
}

/** UTF-16BE code unit `index` of the memorial name, which starts after 8 payload bytes */
static uint16_t memorial_char_at(const char* command, size_t index) {
    const uint8_t* data = encode(command)->command[0].data;
    return (data[3 + 8 + index * 2] << 8) | data[3 + 8 + index * 2 + 1];
}

/**
 * strtok splits the text on its first space, so parseMode copies past the token's
 * terminator once to put that space back. With a single word there is nothing to
 * rejoin and the space is appended anyway -- "Test" becomes "Test ". Known defect,
 * frozen deliberately; hass-divoom writes a null here.
*/
static void test_memorial_single_word_gains_a_trailing_space(void) {
    TEST_ASSERT_EQUAL_UINT16(' ', memorial_char_at("memorial 2 2024-05-06 12:00:00 Test", 4));
    TEST_ASSERT_EQUAL_UINT16(0, memorial_char_at("memorial 2 2024-05-06 12:00:00 Test", 5));

    const char* two = "memorial 2 2024-05-06 12:00:00 Two Words";
    TEST_ASSERT_EQUAL_UINT16(' ', memorial_char_at(two, 3));
    TEST_ASSERT_EQUAL_UINT16('W', memorial_char_at(two, 4));
}

/**
 * parseMode always hands show_memorial a local char[17], never nullptr, so the
 * "ESP32" default is unreachable and a missing text yields 16 null characters.
*/
static void test_memorial_without_text_emits_empty_name(void) {
    data_commands_t* commands = encode("memorial 2 2024-05-06 12:00:00");
    const data_command_t* frame = &commands->command[0];

    TEST_ASSERT_EQUAL_UINT8(1, commands->count);
    for (size_t i = 0; i < 32; i++) {
        TEST_ASSERT_EQUAL_HEX8(0x00, frame->data[3 + 8 + i]);
    }
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_framing_invariant_over_all_cases);
    RUN_TEST(test_checksum_high_byte_is_little_endian);
    RUN_TEST(test_volume_is_scaled_to_fifteen_steps);
    RUN_TEST(test_frame_counts_depend_on_arguments);
    RUN_TEST(test_prefix_without_argument_emits_nothing);
    RUN_TEST(test_datetime_falls_back_to_zeroes_when_time_is_short);
    RUN_TEST(test_alarm_falls_back_to_off_when_time_is_short);
    RUN_TEST(test_alarm_weekdays_are_ignored);
    RUN_TEST(test_scoreboard_with_unknown_version_emits_nothing);
    RUN_TEST(test_memorial_single_word_gains_a_trailing_space);
    RUN_TEST(test_memorial_without_text_emits_empty_name);
    return UNITY_END();
}
