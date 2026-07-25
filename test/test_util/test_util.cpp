#include <unity.h>

#include <limits.h>

#include "util.h"

void setUp(void) {}
void tearDown(void) {}

static void test_getelapsed_returns_difference(void) {
    fake_millis = 1000;
    TEST_ASSERT_EQUAL_UINT32(250, getElapsed(750));
}

static void test_getelapsed_returns_zero_for_now(void) {
    fake_millis = 4711;
    TEST_ASSERT_EQUAL_UINT32(0, getElapsed(fake_millis));
}

/**
 * millis() wraps after about 49 days on the ESP32. The unsigned subtraction has
 * to keep working across that point. ULONG_MAX rather than a fixed constant,
 * because unsigned long is 32 bit there and 64 bit on the test host.
*/
static void test_getelapsed_survives_millis_wraparound(void) {
    fake_millis = 5;
    TEST_ASSERT_EQUAL_UINT32(10, getElapsed(ULONG_MAX - 4));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_getelapsed_returns_difference);
    RUN_TEST(test_getelapsed_returns_zero_for_now);
    RUN_TEST(test_getelapsed_survives_millis_wraparound);
    return UNITY_END();
}
