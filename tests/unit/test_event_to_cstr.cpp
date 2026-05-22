// Tests for ulog_event_to_cstr / ulog_event_to_cstr_colored with a user-defined
// static config header (ulog_config_color_level.h):
//   ULOG_BUILD_DYNAMIC_CONFIG  0
//   ULOG_BUILD_SOURCE_LOCATION 0
//   ULOG_BUILD_LEVEL_SHORT     1
//   ULOG_BUILD_COLOR           1
//   ULOG_BUILD_TIME            1

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

extern "C" {
#include "ulog.h"
#include "ut_callback.h"
}

#include <cctype>
#include <cstring>

struct Fixture {
    Fixture() {
        ulog_cleanup();
        ut_callback_reset();
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
    }
    ~Fixture() { ulog_cleanup(); }
};

struct ColoredFixture {
    ColoredFixture() {
        ulog_cleanup();
        ut_callback_reset();
        ulog_output_add(ut_callback_colored, nullptr, ULOG_LEVEL_TRACE);
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
    }
    ~ColoredFixture() { ulog_cleanup(); }
};

// ULOG_BUILD_COLOR 1 → ANSI codes must appear when using ulog_event_to_cstr_colored
TEST_CASE_FIXTURE(ColoredFixture, "Event to cstr - ulog_event_to_cstr_colored emits ANSI codes when ULOG_BUILD_COLOR=1") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strchr(msg, '\x1b') != nullptr);  // ANSI ESC must appear
}

// ULOG_BUILD_SOURCE_LOCATION 0 → source location must not appear
TEST_CASE_FIXTURE(Fixture, "Event to cstr - ULOG_BUILD_SOURCE_LOCATION=0 respected in static config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strstr(msg, ".cpp:") == nullptr);
}

// ULOG_BUILD_LEVEL_SHORT 1 → short level names must appear
TEST_CASE_FIXTURE(Fixture, "Event to cstr - ULOG_BUILD_LEVEL_SHORT=1 respected in static config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "I ") != nullptr);
    CHECK(strstr(msg, "INFO") == nullptr);
}

// ULOG_BUILD_TIME 1 → timestamp must appear
TEST_CASE_FIXTURE(Fixture, "Event to cstr - ULOG_BUILD_TIME=1 respected in static config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);

    bool has_timestamp = false;
    for (int i = 0; msg[i] && msg[i + 1] && msg[i + 2] && msg[i + 3] &&
                    msg[i + 4] && msg[i + 5] && msg[i + 6] && msg[i + 7];
         i++) {
        if (isdigit((unsigned char)msg[i]) &&
            isdigit((unsigned char)msg[i + 1]) && msg[i + 2] == ':' &&
            isdigit((unsigned char)msg[i + 3]) &&
            isdigit((unsigned char)msg[i + 4]) && msg[i + 5] == ':' &&
            isdigit((unsigned char)msg[i + 6]) &&
            isdigit((unsigned char)msg[i + 7])) {
            has_timestamp = true;
            break;
        }
    }
    CHECK(has_timestamp);
}
