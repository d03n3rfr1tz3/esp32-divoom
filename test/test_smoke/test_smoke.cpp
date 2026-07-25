#include <unity.h>

#include "divoom/divoom.h"
#include "util.h"

void setUp(void) {}
void tearDown(void) {}

/**
 * parseMode mutates its buffer via strtok, so never pass a string literal --
 * always a writable copy.
 *
 * `Divoom d = Divoom()` value-initializes the instance. A plain `Divoom d;`
 * would leave commands.count indeterminate (the class has no constructor),
 * causing a write past the command[5] array. Both production callers do the
 * same, see src/input/serial.cpp:136 and src/input/mqtt.cpp:228.
*/
static void test_parsemode_links_and_emits_one_command(void) {
    char buffer[64] = { 0 };
    strcpy(buffer, "brightness 42");

    Divoom divoom = Divoom();
    data_commands_t* commands = divoom.parseMode(buffer, strlen(buffer));

    TEST_ASSERT_NOT_NULL(commands);
    TEST_ASSERT_EQUAL_UINT8(1, commands->count);
    TEST_ASSERT_EQUAL_size_t(8, commands->command[0].size);
}

/** also proves util.cpp is built and the millis() stub links */
static void test_getelapsed_links(void) {
    fake_millis = 1000;
    TEST_ASSERT_EQUAL_UINT32(250, getElapsed(750));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_parsemode_links_and_emits_one_command);
    RUN_TEST(test_getelapsed_links);
    return UNITY_END();
}
