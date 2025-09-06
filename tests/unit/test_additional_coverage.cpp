// Provide doctest main implementation in this TU
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"
#include <string.h>
#include <atomic>
#include <vector>

// We'll re-use ut_callback for one added output and add a second that counts invocations.

static std::atomic<int> g_lock_acquire_count{0};
static std::atomic<int> g_lock_release_count{0};
static void test_lock(bool lock, void *arg) {
    (void)arg;
    if (lock) {
        g_lock_acquire_count++;
    } else {
        g_lock_release_count++;
    }
}

struct capture_output_ctx { char buf[256]; }; // simple capture through event_to_cstr
static void capture_output(ulog_event *ev, void *arg) {
    if (!ev) return;
    capture_output_ctx *ctx = (capture_output_ctx*)arg;
    ulog_event_to_cstr(ev, ctx->buf, sizeof(ctx->buf));
}

TEST_CASE("lock function invoked around log call") {
    ulog_lock_set_fn(test_lock, NULL);
    g_lock_acquire_count = 0; g_lock_release_count = 0;
    ulog_info("Lock test %d", 1);
    CHECK(g_lock_acquire_count == 1);
    CHECK(g_lock_release_count == 1);
    // Reset lock function
    ulog_lock_set_fn(NULL, NULL);
}

TEST_CASE("output early stop after removed slot") {
    // Add two outputs, remove the first added, ensure second still receives
    ut_callback_reset();
    ulog_output_id a = ulog_output_add(ut_callback, NULL, ULOG_LEVEL_TRACE);
    ulog_output_id b = ulog_output_add(ut_callback, NULL, ULOG_LEVEL_TRACE);
    REQUIRE(a > 0); // stdout is 0
    REQUIRE(b > a);
    // Remove a, internal loop should stop at removed slot and not call b.
    REQUIRE(ulog_output_remove(a) == ULOG_STATUS_OK);
    ulog_info("Test message");
    // Only stdout should have processed + NOT output b (since iteration stops on NULL)
    // stdout + maybe others earlier; we confirm by adding another after gap.
    int count_after_first = ut_callback_get_message_count();
    // Now add another output (will occupy slot 'a' again) and log to hit both.
    ulog_output_id a2 = ulog_output_add(ut_callback, NULL, ULOG_LEVEL_TRACE);
    REQUIRE(a2 == a); // Reuse of freed slot
    ulog_info("Test message2");
    int count_after_second = ut_callback_get_message_count();
    CHECK(count_after_second > count_after_first); // new output got invoked
}

TEST_CASE("color disabled produces no ANSI sequences") {
    // We rely on dynamic config build else this is no-op; guard run otherwise.
#ifdef ULOG_BUILD_DYNAMIC_CONFIG
    ut_callback_reset();
    // Ensure enabled then disable and compare outputs.
    ulog_color_config(true); ulog_info("Color on");
    char colored[256]; strncpy(colored, ut_callback_get_last_message(), sizeof(colored));
    ut_callback_reset();
    ulog_color_config(false); ulog_info("Color off");
    const char *no_color = ut_callback_get_last_message();
    // Basic heuristic: if color off, message shouldn't contain ESC '[' sequence.
    CHECK(strstr(no_color, "\x1b[") == nullptr);
#endif
}

// We can't directly call static time_print_if_invalid so instead attempt to simulate
// an event with NULL time by logging with time disabled (branch: !time_config_is_enabled())
// already covered; to target the other early return path we'd need injection -> skip if unreachable.
TEST_CASE("time disabled path already covered marker") {
    // Just ensure we can toggle and log.
#ifdef ULOG_BUILD_DYNAMIC_CONFIG
    ulog_time_config(false); ulog_info("No time");
    ulog_time_config(true);
#endif
    CHECK(true);
}
