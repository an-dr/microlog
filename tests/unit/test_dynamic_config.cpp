#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include "ulog.h"
#include "ut_callback.h"

static void test_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PREFIX]");
}

struct TestFixture {
    TestFixture() {
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ut_callback_reset();
        ulog_prefix_set_fn(test_prefix);
    }
    ~TestFixture() = default;
};

TEST_CASE_FIXTURE(TestFixture, "Dynamic Config - Prefix") {
    // Enable prefix
    REQUIRE(ulog_prefix_config(true) == ULOG_STATUS_OK);

    // Log a message
    ulog_info("Test message with prefix");

    // Check the last callback message
    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[PREFIX]") != nullptr);
    REQUIRE(strstr(last_message, "Test message with prefix") != nullptr);

    REQUIRE(ulog_prefix_config(false) == ULOG_STATUS_OK);

    // Log a message
    ulog_info("Test message without prefix");

    // Check the last callback message
    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[PREFIX]") == nullptr);
    REQUIRE(strstr(last_message, "Test message without prefix") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Dynamic Config - Color") {
    // Test color enable/disable
    REQUIRE(ulog_color_config(true) == ULOG_STATUS_OK);
    ulog_error("Test message with color");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "Test message with color") != nullptr);

    REQUIRE(ulog_color_config(false) == ULOG_STATUS_OK);
    ulog_error("Test message without color");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "Test message without color") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Dynamic Config - File String") {
    // Test file string enable/disable
    REQUIRE(ulog_source_location_config(true) == ULOG_STATUS_OK);
    ulog_info("Test message with file string");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "test_dynamic_config.cpp") != nullptr);
    REQUIRE(strstr(last_message, "Test message with file string") != nullptr);

    REQUIRE(ulog_source_location_config(false) == ULOG_STATUS_OK);
    ulog_info("Test message without file string");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "test_dynamic_config.cpp") == nullptr);
    REQUIRE(strstr(last_message, "Test message without file string") !=
            nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Dynamic Config - Short Level Strings") {
    // Test short level strings enable/disable
    REQUIRE(ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_SHORT) == ULOG_STATUS_OK);
    ulog_info("Test message with short level");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "I ") != nullptr);  // Short for INFO
    REQUIRE(strstr(last_message, "Test message with short level") != nullptr);

    REQUIRE(ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_DEFAULT) == ULOG_STATUS_OK);
    ulog_info("Test message with long level");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "INFO ") != nullptr);  // Long form
    REQUIRE(strstr(last_message, "Test message with long level") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Dynamic Config - Time") {
    // Test time enable/disable
    REQUIRE(ulog_time_config(true) == ULOG_STATUS_OK);
    ulog_warn("Test message with time");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    // Check for time format HH:MM:SS (should contain colons)
    bool has_time = (strchr(last_message, ':') != nullptr) &&
                    (strrchr(last_message, ':') != strchr(last_message, ':'));
    REQUIRE(has_time);
    REQUIRE(strstr(last_message, "Test message with time") != nullptr);

    REQUIRE(ulog_time_config(false) == ULOG_STATUS_OK);
    ulog_warn("Test message without time");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "Test message without time") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Dynamic Config - Topics") {
    // Add a test topic first
    ulog_topic_add("test_dynamic_cfg_topic", ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);

    // Test topics enable/disable
    REQUIRE(ulog_topic_config(true) == ULOG_STATUS_OK);
    ulog_t_info("test_dynamic_cfg_topic", "Test message with topic shown");

    const char *last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[test_dynamic_cfg_topic]") != nullptr);
    REQUIRE(strstr(last_message, "Test message with topic shown") != nullptr);

    REQUIRE(ulog_topic_config(false) == ULOG_STATUS_OK);
    ulog_t_info("test_dynamic_cfg_topic", "Test message with topic hidden");

    last_message = ut_callback_get_last_message();
    REQUIRE(last_message != nullptr);
    REQUIRE(strstr(last_message, "[test_dynamic_cfg_topic]") == nullptr);
    REQUIRE(strstr(last_message, "Test message with topic hidden") != nullptr);
}
