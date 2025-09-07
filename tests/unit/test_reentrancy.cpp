#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include "ulog.h"
#include "ut_callback.h"

// Prefix function that logs once (reentrancy test for prefix path)
static void reentrant_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    static bool prefix_logged = false;
    snprintf(prefix, prefix_size, "[PFX]");
    if (!prefix_logged) {
        prefix_logged = true; // Guard to avoid infinite recursion
        ulog_debug("Prefix side log");
    }
}

// Output callback that logs once from inside an output (reentrancy test for output path)
static void reentrant_output(ulog_event *ev, void *arg) {
    (void)arg;
    static bool nested_done = false;
    // Only trigger nested log for the first INFO level event to keep deterministic
    if (!nested_done && ulog_event_get_level(ev) == ULOG_LEVEL_INFO) {
        nested_done = true; // Guard
        ulog_warn("Nested from output");
    }
}

// Simple lock function with simple locking
static ulog_status lock_fn(bool lock, void *arg) {
    (void)arg;
    static bool is_locked = false;
    if (lock) {
        REQUIRE(!is_locked); // Should not already be locked
        is_locked = true;
    return ULOG_STATUS_OK;
    } else {
        REQUIRE(is_locked); // Should be locked when unlocking
        is_locked = false;
    return ULOG_STATUS_OK;
    }
}


struct ReentrancyFixture {
    ReentrancyFixture() {
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        // Primary capture output
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        // Reentrant testing output
        ulog_output_add(reentrant_output, nullptr, ULOG_LEVEL_TRACE);
        ulog_lock_set_fn(lock_fn, nullptr); // No locking
        ut_callback_reset();
        ulog_prefix_set_fn(reentrant_prefix);
    }
    ~ReentrancyFixture() = default;
};

TEST_CASE_FIXTURE(ReentrancyFixture, "Reentrancy: prefix + output nesting") {
    ulog_info("Outer message");

    // Expected events:
    // 1) Prefix side log (DEBUG) from prefix function
    // 2) Outer message (INFO)
    // 3) Nested from output (WARN) from output callback
    CHECK(ut_callback_get_message_count() == 3);
    const char *last = ut_callback_get_last_message();
    REQUIRE(last != nullptr);
    CHECK(strstr(last, "Nested from output") != nullptr);
}
