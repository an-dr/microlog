#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstring>
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"

static void test_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PREFIX]");
}

struct TestFixture {
    TestFixture() {
        ulog_output_set_level(ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ut_callback_reset();
        ulog_prefix_set_fn(test_prefix);
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

    ulog_configure_prefix(false);

    // Log a message
    log_info("Test message without custom prefix");

    // Check the last callback message
    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[PREFIX]") == nullptr);
    REQUIRE(strstr(last_message, "Test message without custom prefix") !=
            nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Runtime Config - Color") {
    // Test color enable/disable
    ulog_configure_color(true);
    log_error("Test message with color");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "Test message with color") != nullptr);

    ulog_configure_color(false);
    log_error("Test message without color");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "Test message without color") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Runtime Config - File String") {
    // Test file string enable/disable
    ulog_configure_file_string(true);
    log_info("Test message with file string");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "test_runtime_config.cpp") != nullptr);
    REQUIRE(strstr(last_message, "Test message with file string") != nullptr);

    ulog_configure_file_string(false);
    log_info("Test message without file string");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "test_runtime_config.cpp") == nullptr);
    REQUIRE(strstr(last_message, "Test message without file string") !=
            nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Runtime Config - Short Level Strings") {
    // Test short level strings enable/disable
    ulog_configure_levels(true);  // Use correct function name
    log_info("Test message with short level");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "I ") != nullptr);  // Short for INFO
    REQUIRE(strstr(last_message, "Test message with short level") != nullptr);

    ulog_configure_levels(false);  // Use correct function name
    log_info("Test message with long level");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "INFO ") != nullptr);  // Long form
    REQUIRE(strstr(last_message, "Test message with long level") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Runtime Config - Time") {
    // Test time enable/disable
    ulog_configure_time(true);
    log_warn("Test message with time");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    // Check for time format HH:MM:SS (should contain colons)
    bool has_time = (strchr(last_message, ':') != nullptr) &&
                    (strrchr(last_message, ':') != strchr(last_message, ':'));
    REQUIRE(has_time);
    REQUIRE(strstr(last_message, "Test message with time") != nullptr);

    ulog_configure_time(false);
    log_warn("Test message without time");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "Test message without time") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Runtime Config - Topics") {
    // Add a test topic first
    ulog_topic_add("test_runtime_topic", true);

    // Test topics enable/disable
    ulog_configure_topics(true);
    logt_info("test_runtime_topic", "Test message with topic shown");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[test_runtime_topic]") != nullptr);
    REQUIRE(strstr(last_message, "Test message with topic shown") != nullptr);

    ulog_configure_topics(false);
    logt_info("test_runtime_topic", "Test message with topic hidden");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[test_runtime_topic]") == nullptr);
    REQUIRE(strstr(last_message, "Test message with topic hidden") != nullptr);
}
