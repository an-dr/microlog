#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include "TryMutex.hpp"
#include "ulog.h"
#include "ut_callback.h"

static std::vector<std::string> lock_events;
static TryMutex try_mutex;

// Prefix function that logs once (reentrancy test for prefix path)
static void reentrant_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    static bool prefix_logged = false;
    snprintf(prefix, prefix_size, "[PFX]");
    if (!prefix_logged) {
        prefix_logged = true;  // Guard to avoid infinite recursion
        ulog_debug("Prefix side log");
    }
}

// Output callback that logs once from inside an output (reentrancy test for
// output path)
static void reentrant_output(ulog_event *ev, void *arg) {
    (void)arg;
    static bool nested_done = false;
    // Only trigger nested log for the first INFO level event to keep
    // deterministic
    if (!nested_done && ulog_event_get_level(ev) == ULOG_LEVEL_INFO) {
        nested_done = true;  // Guard to avoid infinite recursion
        ulog_warn("Nested from output");
    }
}

// Simple lock function with simple locking
static ulog_status lock_fn(bool lock, void *arg) {
    (void)arg;
    if (lock) {
        if (!try_mutex.try_lock()) {
            return ULOG_STATUS_BUSY;  // Already locked
        }
        lock_events.push_back("lock");

    } else {
        try_mutex.unlock();
        lock_events.push_back("unlock");
    }
    return ULOG_STATUS_OK;
}

struct LockingTestFixture {
    LockingTestFixture() {
        lock_events.clear();
        try_mutex.unlock();
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        // Primary capture output
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        // Reentrant testing output
        ulog_output_add(reentrant_output, nullptr, ULOG_LEVEL_TRACE);
        ulog_lock_set_fn(lock_fn, nullptr);  // No locking
        ut_callback_reset();
        ulog_prefix_set_fn(reentrant_prefix);
    }
    ~LockingTestFixture() = default;
};

TEST_CASE_FIXTURE(LockingTestFixture, "Thread Safety: Locking") {
    ulog_info("Lock test");
    CHECK(std::count(lock_events.begin(), lock_events.end(), "lock") >= 1);
    CHECK(std::count(lock_events.begin(), lock_events.end(), "unlock") >= 1);
}

TEST_CASE_FIXTURE(LockingTestFixture, "Reentrancy: prefix + output nesting") {
    ulog_info("Outer message");

    // Expected events:
    // 1) Prefix side log (DEBUG) from prefix function
    // 2) Outer message (INFO)
    // not 3) Nested from output (WARN) from output callback
    CHECK(ut_callback_get_message_count() == 2);
    const char *last = ut_callback_get_last_message();
    REQUIRE(last != nullptr);
    CHECK(strstr(last, "Nested from output") == nullptr);
}
