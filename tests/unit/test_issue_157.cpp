// Regression tests for GitHub issue #157 — dynamic config mode.
//
// These tests verify that the runtime ulog_*_config() API correctly
// affects what custom handlers receive via ulog_event_to_cstr().
// Compiled with ULOG_BUILD_DYNAMIC_CONFIG=1 so all features are compiled in
// and controlled at runtime.

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

// ulog_event_to_cstr must forward the runtime colour state to custom handlers
// (heeplr confirmed: ulog_event_to_cstr was hardcoding color=false, discarding
//  ANSI codes even after ulog_color_config(true))
TEST_CASE_FIXTURE(Fixture, "Issue157 heeplr - ulog_event_to_cstr forwards colour state to custom handler") {
    ulog_color_config(true);
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strchr(msg, '\x1b') != nullptr);  // ANSI ESC must appear
}

// ulog_level_config(SHORT) must be reflected in custom handler output
TEST_CASE_FIXTURE(Fixture, "Issue157 - ulog_level_config SHORT reflected in custom handler") {
    ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_SHORT);
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "I ") != nullptr);    // short level name must appear
    CHECK(strstr(msg, "INFO") == nullptr);  // long level name must NOT appear
}

// ulog_source_location_config(false) must suppress file:line in custom handler
TEST_CASE_FIXTURE(Fixture, "Issue157 - ulog_source_location_config(false) respected in custom handler") {
    ulog_source_location_config(false);
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strstr(msg, ".cpp:") == nullptr);  // source location must NOT appear
}

// ulog_time_config(false) must suppress the timestamp in custom handler
TEST_CASE_FIXTURE(Fixture, "Issue157 - ulog_time_config(false) respected in custom handler") {
    ulog_time_config(false);
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
    CHECK(!has_timestamp);  // timestamp must NOT appear
}
