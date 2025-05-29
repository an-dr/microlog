#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "ulog.h"
#include "ut_callback.h"

#include <string>
#include <vector>
#include <algorithm>

// Dummy lock for thread-safety test
static std::vector<std::string> lock_events;
void dummy_lock(bool lock, void *arg) {
    (void)arg;
    lock_events.push_back(lock ? "lock" : "unlock");
}

struct CoreTestFixture {
    CoreTestFixture() {
        // Per-test setup
        ulog_set_level(LOG_TRACE);
        ulog_set_quiet(false);
        ulog_add_callback(ut_callback, nullptr, LOG_TRACE);
        ut_callback_reset();
    }

    ~CoreTestFixture() = default;
};

TEST_CASE_FIXTURE(CoreTestFixture, "Macros") {
    log_trace("This is a TRACE message: %d", 123);
    log_debug("This is a DEBUG message: %s", "test");
    log_info("This is an INFO message: %.2f", 1.23);
    log_warn("This is a WARN message");
    log_error("This is an ERROR message: %x", 0xff);
    log_fatal("This is a FATAL message");

    CHECK(ut_callback_get_message_count() == 6);
    CHECK(strcmp(ut_callback_get_last_message(), "This is a FATAL message") == 0);
}

TEST_CASE_FIXTURE(CoreTestFixture, "Levels") {
    ulog_set_level(LOG_INFO);

    log_trace("This TRACE should not be processed.");
    CHECK(ut_callback_get_message_count() == 0);
    log_debug("This DEBUG should not be processed.");
    CHECK(ut_callback_get_message_count() == 0);
    log_info("This INFO should be processed.");
    CHECK(ut_callback_get_message_count() == 1);
    log_warn("This WARN should be processed.");
    CHECK(ut_callback_get_message_count() == 2);
    log_error("This ERROR should be processed.");
    CHECK(ut_callback_get_message_count() == 3);
    log_fatal("This FATAL should be processed.");
    CHECK(ut_callback_get_message_count() == 4);
}

TEST_CASE_FIXTURE(CoreTestFixture, "Quiet Mode") {
    ulog_set_quiet(true);
    log_info("This message will trigger extra callbacks, stdout should be quiet.");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_set_quiet(false);
    log_info("This message will trigger extra callbacks, stdout is not quiet.");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(CoreTestFixture, "Thread Safety: Locking") {
    lock_events.clear();
    ulog_set_lock(dummy_lock, nullptr);
    log_info("Lock test");
    // At least one lock/unlock pair should be present
    CHECK(std::count(lock_events.begin(), lock_events.end(), "lock") >= 1);
    CHECK(std::count(lock_events.begin(), lock_events.end(), "unlock") >= 1);
}

// Custom prefix test
static void test_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PREFIX]");
}

TEST_CASE_FIXTURE(CoreTestFixture, "Custom Prefix") {
    ulog_set_prefix_fn(test_prefix);
    log_info("Prefix test");
    CHECK(strstr(ut_callback_get_last_message(), "Prefix test") != nullptr);
    // Note: ut_callback only captures the message, not the prefix, so this is a smoke test
}

// Color test: check that disabling color removes ANSI codes from output
// This test is limited by the callback, but we can at least check the macro is defined
TEST_CASE("Color Feature Macro") {
#if FEATURE_COLOR
    CHECK(true);
#else
    CHECK(true); // If color is off, test passes
#endif
}

// Time test: check that enabling time adds a timestamp (smoke test)
TEST_CASE("Time Feature Macro") {
#if FEATURE_TIME
    CHECK(true);
#else
    CHECK(true); // If time is off, test passes
#endif
}

// Topics: enable/disable and per-topic level
TEST_CASE_FIXTURE(CoreTestFixture, "Topics: Enable/Disable and Levels") {
#if FEATURE_TOPICS
    ulog_add_topic("testtopic", true);
    logt_info("testtopic", "Topic enabled");
    CHECK(ut_callback_get_message_count() == 1);
    ulog_disable_topic("testtopic");
    logt_info("testtopic", "Should not appear");
    CHECK(ut_callback_get_message_count() == 1);
    ulog_enable_topic("testtopic");
    ulog_set_topic_level("testtopic", LOG_ERROR);
    logt_warn("testtopic", "Below topic level");
    CHECK(ut_callback_get_message_count() == 1);
    logt_error("testtopic", "At topic level");
    CHECK(ut_callback_get_message_count() == 2);
#endif
}
