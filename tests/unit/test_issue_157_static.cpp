// Regression test for GitHub issue #157 — config-header mode.
//
// Compiled with ULOG_BUILD_CONFIG_HEADER_ENABLED=1 pointing at
// ulog_config_issue157.h which contains the user's intended settings:
//   ULOG_BUILD_SOURCE_LOCATION=0   (no file:line prefix)
//   ULOG_BUILD_LEVEL_SHORT=1       (short level names: I, W…)
//   ULOG_BUILD_COLOR=1             (ANSI colour)
//   ULOG_BUILD_TIME=1              (timestamp)
//
// Bugs:
// 1. CONFIG_HEADER_ENABLED forces ULOG_HAS_SOURCE_LOCATION=1, so source
//    location always appears even though the user set it to 0.
// 2. Defining ULOG_BUILD_DYNAMIC_CONFIG in the config header (even as 0)
//    triggers the dynamic config path. level_cfg.short_style is always
//    initialised to false, ignoring ULOG_BUILD_LEVEL_SHORT=1, so long
//    level names appear instead of short ones.
// 3. ulog_event_to_cstr() hardcodes color=false, so no ANSI codes reach
//    the custom handler even though ULOG_BUILD_COLOR=1.
//
// The timestamp test passes — ULOG_HAS_TIME is forced to 1 which matches
// what the user wanted, so there is no mismatch there.
//
// Tests marked "EXPECTED TO FAIL" reproduce the bugs.
// Tests marked "expected to pass" confirm correct behaviour.

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

// ── EXPECTED TO FAIL ─────────────────────────────────────────────────────────
// User set ULOG_BUILD_SOURCE_LOCATION=0 in their config header.
// CONFIG_HEADER_ENABLED forces ULOG_HAS_SOURCE_LOCATION=1, so source location
// always appears regardless.
TEST_CASE_FIXTURE(Fixture, "Issue157 config-header - ULOG_BUILD_SOURCE_LOCATION=0 respected") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strstr(msg, ".cpp:") == nullptr);  // source location must NOT appear
}

// ── EXPECTED TO FAIL ─────────────────────────────────────────────────────────
// User set ULOG_BUILD_LEVEL_SHORT=1 in their config header (short names).
// Defining ULOG_BUILD_DYNAMIC_CONFIG in that header (even as 0) activates the
// dynamic path; level_cfg.short_style is always initialised to false so long
// level names appear instead of the requested short ones.
TEST_CASE_FIXTURE(Fixture, "Issue157 config-header - ULOG_BUILD_LEVEL_SHORT=1 respected") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "I ") != nullptr);    // short level name must appear
    CHECK(strstr(msg, "INFO") == nullptr);  // long level name must NOT appear
}

// ── EXPECTED TO FAIL ─────────────────────────────────────────────────────────
// User set ULOG_BUILD_COLOR=1 in their config header.
// ulog_event_to_cstr() always calls log_print_event with color=false, so no
// ANSI codes reach the custom handler buffer even though stdout gets them.
TEST_CASE_FIXTURE(Fixture, "Issue157 config-header - ULOG_BUILD_COLOR=1 respected by ulog_event_to_cstr") {
    ulog_info("hello");

    const char *msg = ut_callback_get_last_message();
    REQUIRE(msg != nullptr);

    CHECK(strstr(msg, "hello") != nullptr);
    CHECK(strchr(msg, '\x1b') != nullptr);  // ANSI ESC must appear
}

// ── expected to pass ─────────────────────────────────────────────────────────
// User set ULOG_BUILD_TIME=1. CONFIG_HEADER_ENABLED also forces
// ULOG_HAS_TIME=1, which happens to match, so time appears correctly.
TEST_CASE_FIXTURE(Fixture, "Issue157 config-header - ULOG_BUILD_TIME=1 respected by ulog_event_to_cstr") {
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
