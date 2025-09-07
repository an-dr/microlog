#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <algorithm>
#include <string>
#include <vector>
#include "ulog.h"
#include "ut_callback.h"

static std::vector<std::string> lock_events;
ulog_status dummy_lock(bool lock, void *arg) {
    (void)arg;
    lock_events.push_back(lock ? "lock" : "unlock");
    return ULOG_STATUS_OK;
}

struct LockingTestFixture {
    LockingTestFixture() {
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ut_callback_reset();
        ulog_lock_set_fn(dummy_lock, nullptr);
        lock_events.clear();
    }
    ~LockingTestFixture() = default;
};

TEST_CASE_FIXTURE(LockingTestFixture, "Thread Safety: Locking") {
    ulog_info("Lock test");
    CHECK(std::count(lock_events.begin(), lock_events.end(), "lock") >= 1);
    CHECK(std::count(lock_events.begin(), lock_events.end(), "unlock") >= 1);
}
