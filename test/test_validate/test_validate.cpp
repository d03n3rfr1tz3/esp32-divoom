#include <unity.h>

#include "validate.h"

void setUp(void) {}
void tearDown(void) {}

static void test_topic_accepts_the_default_pattern(void) {
    TEST_ASSERT_TRUE(isValidTopicFormat("divoom/%s"));
}

static void test_topic_accepts_a_nested_pattern(void) {
    TEST_ASSERT_TRUE(isValidTopicFormat("divoom/livingroom/%s"));
}

static void test_topic_accepts_a_leading_placeholder(void) {
    TEST_ASSERT_TRUE(isValidTopicFormat("%s/divoom"));
}

static void test_topic_rejects_a_missing_placeholder(void) {
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/"));
}

/**
 * the pattern ends up as the format string of snprintf, so anything but a single
 * %s is undefined behaviour. %n is the dangerous one, it writes through the
 * argument instead of reading from it.
*/
static void test_topic_rejects_other_conversions(void) {
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%d"));
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%n"));
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%d%s"));
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%x%s"));
}

static void test_topic_rejects_multiple_placeholders(void) {
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%s/%s"));
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%s%s"));
}

static void test_topic_rejects_a_trailing_percent(void) {
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%s/%"));
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%"));
}

static void test_topic_rejects_an_escaped_percent(void) {
    TEST_ASSERT_FALSE(isValidTopicFormat("divoom/%%/%s"));
}

/**
 * the formatted topic has to fit into the char[64] buffers of input/mqtt.h,
 * together with the longest suffix the firmware appends
*/
static void test_topic_rejects_an_overlong_pattern(void) {
    TEST_ASSERT_EQUAL_UINT32(40, strlen("0123456789012345678901234567890123456/%s"));
    TEST_ASSERT_TRUE(isValidTopicFormat("0123456789012345678901234567890123456/%s"));
    TEST_ASSERT_EQUAL_UINT32(41, strlen("01234567890123456789012345678901234567/%s"));
    TEST_ASSERT_FALSE(isValidTopicFormat("01234567890123456789012345678901234567/%s"));
}

static void test_topic_rejects_empty_input(void) {
    TEST_ASSERT_FALSE(isValidTopicFormat(""));
    TEST_ASSERT_FALSE(isValidTopicFormat(NULL));
}

static void test_hostname_accepts_the_default_name(void) {
    TEST_ASSERT_TRUE(isValidHostname("ESP32-Divoom"));
    TEST_ASSERT_TRUE(isValidHostname("divoom-2"));
    TEST_ASSERT_TRUE(isValidHostname("a"));
}

static void test_hostname_rejects_forbidden_characters(void) {
    TEST_ASSERT_FALSE(isValidHostname("Divoom_1"));
    TEST_ASSERT_FALSE(isValidHostname("a.b"));
    TEST_ASSERT_FALSE(isValidHostname("my divoom"));
    TEST_ASSERT_FALSE(isValidHostname("divoom/1"));
}

static void test_hostname_rejects_a_surrounding_hyphen(void) {
    TEST_ASSERT_FALSE(isValidHostname("-divoom"));
    TEST_ASSERT_FALSE(isValidHostname("divoom-"));
}

static void test_hostname_rejects_an_overlong_name(void) {
    TEST_ASSERT_EQUAL_UINT32(31, strlen("0123456789012345678901234567890"));
    TEST_ASSERT_TRUE(isValidHostname("0123456789012345678901234567890"));
    TEST_ASSERT_EQUAL_UINT32(32, strlen("01234567890123456789012345678901"));
    TEST_ASSERT_FALSE(isValidHostname("01234567890123456789012345678901"));
}

static void test_hostname_rejects_empty_input(void) {
    TEST_ASSERT_FALSE(isValidHostname(""));
    TEST_ASSERT_FALSE(isValidHostname(NULL));
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_topic_accepts_the_default_pattern);
    RUN_TEST(test_topic_accepts_a_nested_pattern);
    RUN_TEST(test_topic_accepts_a_leading_placeholder);
    RUN_TEST(test_topic_rejects_a_missing_placeholder);
    RUN_TEST(test_topic_rejects_other_conversions);
    RUN_TEST(test_topic_rejects_multiple_placeholders);
    RUN_TEST(test_topic_rejects_a_trailing_percent);
    RUN_TEST(test_topic_rejects_an_escaped_percent);
    RUN_TEST(test_topic_rejects_an_overlong_pattern);
    RUN_TEST(test_topic_rejects_empty_input);
    RUN_TEST(test_hostname_accepts_the_default_name);
    RUN_TEST(test_hostname_rejects_forbidden_characters);
    RUN_TEST(test_hostname_rejects_a_surrounding_hyphen);
    RUN_TEST(test_hostname_rejects_an_overlong_name);
    RUN_TEST(test_hostname_rejects_empty_input);
    return UNITY_END();
}
