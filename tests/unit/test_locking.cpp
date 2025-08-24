#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"
#include <string>
#include <vector>
#include <algorithm>

static std::vector<std::string> lock_events;
void dummy_lock(bool lock, void *arg) {
    (void)arg;
    lock_events.push_back(lock ? "lock" : "unlock");
}

struct LockingTestFixture {
    LockingTestFixture() {
        ulog_set_level(ULOG_TRACE);
        ulog_set_quiet(false);
        ulog_add_callback(ut_callback, nullptr, ULOG_TRACE);
        ut_callback_reset();
        ulog_set_lock(dummy_lock, nullptr);
        lock_events.clear();
    }
    ~LockingTestFixture() = default;
};

TEST_CASE_FIXTURE(LockingTestFixture, "Thread Safety: Locking") {
    log_info("Lock test");
    CHECK(std::count(lock_events.begin(), lock_events.end(), "lock") >= 1);
    CHECK(std::count(lock_events.begin(), lock_events.end(), "unlock") >= 1);
}
