// Regression tests for saphieron's report in GitHub issue #157.
//
// Setup (ulog_config_issue157.h, mirroring saphieron's ulog_config.h):
//   ULOG_BUILD_DYNAMIC_CONFIG  0  — user intends static (non-dynamic) config
//   ULOG_BUILD_SOURCE_LOCATION 0  — no file:line prefix
//   ULOG_BUILD_LEVEL_SHORT     1  — short level names (I, W, E, …)
//   ULOG_BUILD_COLOR           1  — ANSI colour
//   ULOG_BUILD_TIME            1  — timestamp
//
// Root bug: the old check `#ifndef ULOG_BUILD_DYNAMIC_CONFIG` treated
// `ULOG_BUILD_DYNAMIC_CONFIG 0` as "macro is defined → activate dynamic mode."
// That forced every ULOG_HAS_* to 1 regardless of the user's settings.
//
// Fix: changed to `#if !defined(ULOG_BUILD_DYNAMIC_CONFIG) || !(ULOG_BUILD_DYNAMIC_CONFIG)`
// so that defining the macro as 0 correctly keeps static mode active.
//
// All tests in this file expect to PASS after the fix.

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

// ULOG_BUILD_COLOR 1 → ANSI codes must appear in ulog_event_to_cstr output
// (heeplr: ulog_event_to_cstr was hardcoding color=false; now uses color_config_is_enabled())
TEST_CASE_FIXTURE(Fixture, "Issue157 heeplr - ULOG_BUILD_COLOR=1 respected by ulog_event_to_cstr in static config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strchr(msg, '\x1b') != nullptr);  // ANSI ESC must appear
}

// ULOG_BUILD_SOURCE_LOCATION 0 → source location must not appear
TEST_CASE_FIXTURE(Fixture, "Issue157 saphieron - ULOG_BUILD_SOURCE_LOCATION=0 respected in static config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strstr(msg, ".cpp:") == nullptr);  // source location must NOT appear
}

// ULOG_BUILD_LEVEL_SHORT 1 → short level names must appear
TEST_CASE_FIXTURE(Fixture, "Issue157 saphieron - ULOG_BUILD_LEVEL_SHORT=1 respected in static config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "I ") != nullptr);    // short level name must appear
    CHECK(strstr(msg, "INFO") == nullptr);  // long level name must NOT appear
}

// ULOG_BUILD_TIME 1 → timestamp must appear
TEST_CASE_FIXTURE(Fixture, "Issue157 saphieron - ULOG_BUILD_TIME=1 respected in static config") {
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
    CHECK(has_timestamp);  // timestamp must appear
}
