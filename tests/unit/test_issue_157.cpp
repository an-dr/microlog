// Regression test for GitHub issue #157:
// "Manual outputs added via ulog_output_add ignore build options
//  such as ULOG_BUILD_SOURCE_LOCATION or ULOG_BUILD_LEVEL_SHORT"
//
// This test is compiled with ULOG_BUILD_DYNAMIC_CONFIG=1 together with the
// build options the reporter set:
//   ULOG_BUILD_LEVEL_SHORT=1      (short level names: I/W/E/...)
//   ULOG_BUILD_COLOR=1            (ANSI color output enabled)
//   ULOG_BUILD_SOURCE_LOCATION=0  (file:line prefix disabled)
//   ULOG_BUILD_TIME=0             (timestamp disabled)
//
// Bug 1: level_cfg.short_style is always initialized to false regardless of
//        ULOG_BUILD_LEVEL_SHORT, so long level names appear even when the
//        user compiled with ULOG_BUILD_LEVEL_SHORT=1.
//
// Bug 2: ulog_event_to_cstr() always calls log_print_event with color=false,
//        so custom handlers never receive ANSI color codes even when
//        ulog_color_config(true) has been called.
//
// Tests marked "EXPECTED TO FAIL" reproduce the bugs; tests marked
// "expected to pass" confirm behaviors that work correctly today.

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

// ── EXPECTED TO FAIL (Bug 1) ─────────────────────────────────────────────────
// With ULOG_BUILD_LEVEL_SHORT=1, the initial dynamic config should reflect the
// build option — level names must be short without any explicit runtime call.
// Fails because level_cfg.short_style is hardcoded to false at init.
TEST_CASE_FIXTURE(Fixture, "Issue157 - ULOG_BUILD_LEVEL_SHORT=1 reflected in initial dynamic config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strstr(msg, "I ") != nullptr);    // short level name must appear
    CHECK(strstr(msg, "INFO") == nullptr);  // long level name must NOT appear
}

// ── EXPECTED TO FAIL (Bug 2) ─────────────────────────────────────────────────
// ulog_event_to_cstr() hardcodes color=false in its log_print_event call, so
// ANSI escape codes are never written into the custom handler buffer even after
// ulog_color_config(true) is called.
TEST_CASE_FIXTURE(Fixture, "Issue157 - ulog_event_to_cstr respects color config in custom output") {
    ulog_color_config(true);
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strchr(msg, '\x1b') != nullptr);  // ANSI ESC must appear
}

// ── expected to pass ─────────────────────────────────────────────────────────
// ULOG_BUILD_SOURCE_LOCATION=0 is correctly reflected in the initial dynamic
// config (src_loc_cfg.enabled is initialized from ULOG_HAS_SOURCE_LOCATION).
TEST_CASE_FIXTURE(Fixture, "Issue157 - ULOG_BUILD_SOURCE_LOCATION=0 reflected in initial dynamic config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strstr(msg, ".cpp:") == nullptr);  // source location must NOT appear
}

// ── expected to pass ─────────────────────────────────────────────────────────
// ULOG_BUILD_TIME=0 is correctly reflected in the initial dynamic config
// (time_cfg.enabled is initialized from ULOG_HAS_TIME).
TEST_CASE_FIXTURE(Fixture, "Issue157 - ULOG_BUILD_TIME=0 reflected in initial dynamic config") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);

    // Scan for HH:MM:SS pattern
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
