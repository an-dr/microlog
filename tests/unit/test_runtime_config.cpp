#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"
#include <cstring>

static void test_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PREFIX]");
}

struct TestFixture {
    TestFixture() {
        ulog_set_level(LOG_TRACE);
        ulog_set_quiet(false);
        ulog_add_callback(ut_callback, nullptr, LOG_TRACE);
        ut_callback_reset();
        ulog_set_prefix_fn(test_prefix);
    }
    ~TestFixture() = default;
};

TEST_CASE_FIXTURE(TestFixture, "Runtime Config - Custom Prefix") {
    // Enable custom prefix
    ulog_configure_prefix(true);
    
    // Log a message
    log_info("Test message with custom prefix");

    // Check the last callback message
    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[PREFIX]") != nullptr);
    REQUIRE(strstr(last_message, "Test message with custom prefix") != nullptr);
    
}

TEST_CASE_FIXTURE(TestFixture, "Runtime Config - Disable Custom Prefix") {
    // Disable custom prefix
    ulog_configure_prefix(false);
    
    // Log a message
    log_info("Test message without custom prefix");

    // Check the last callback message
    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[PREFIX]") == nullptr);
    REQUIRE(strstr(last_message, "Test message without custom prefix") != nullptr);
}
