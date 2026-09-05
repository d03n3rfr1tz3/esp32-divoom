#include <unity.h>

#include <string>

#include "cases.h"
#include "support.h"
#include "divoom.h"

void setUp(void) {}
void tearDown(void) {}

/** set before each Unity run so the shared test function knows its case */
static const DivoomCase* current_case = NULL;

static std::string encode(const DivoomCase* testCase) {
    // parseMode mutates its buffer via strtok, so it gets a writable copy
    char buffer[256] = { 0 };
    snprintf(buffer, sizeof(buffer), "%s", testCase->command);

    // A fresh, value-initialized instance per case: parseMode never resets
    // commands.count, and Divoom has no constructor that would zero it.
    Divoom divoom = Divoom();
    return format_commands(divoom.parseMode(buffer, strlen(buffer)));
}

static void test_golden(void) {
    std::string actual = encode(current_case);

#ifdef UPDATE_GOLDENS
    std::string message = "cannot write " + golden_path(current_case->name, false);
    TEST_ASSERT_TRUE_MESSAGE(write_golden(current_case->name, actual), message.c_str());
#else
    std::string expected;
    if (!read_golden(current_case->name, expected)) {
        std::string message = "golden file missing: " + golden_path(current_case->name, false)
                            + " -- run `pio test -e record` to generate it";
        TEST_FAIL_MESSAGE(message.c_str());
    }

    TEST_ASSERT_EQUAL_STRING(expected.c_str(), actual.c_str());
#endif
}

int main(int, char**) {
    UNITY_BEGIN();

    for (size_t i = 0; i < CASE_COUNT; i++) {
        current_case = &CASES[i];
        UnityDefaultTestRun(test_golden, current_case->name, __LINE__);
    }

    return UNITY_END();
}
