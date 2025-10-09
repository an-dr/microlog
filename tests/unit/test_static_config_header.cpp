#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "ulog.h"
#include "ut_callback.h"

static void test_prefix_handler(ulog_event *ev, char *prefix,
                                size_t prefix_size) {
    (void)ev;  // Unused parameter
    snprintf(prefix, prefix_size, "[PREFIX] ");
}

static void combined_prefix_handler(ulog_event *ev, char *prefix,
                                    size_t prefix_size) {
    (void)ev;  // Unused parameter
    snprintf(prefix, prefix_size, "[TEST] ");
}

struct TestFixture {
  public:
    TestFixture() {
        // Clean up any previous state
        ulog_cleanup();
        ut_callback_reset();

        // Set up test environment
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
    }

    ~TestFixture() {
        // Clean up after each test
        ulog_cleanup();
    }
};

TEST_CASE_FIXTURE(TestFixture, "Static Config Header - Basic Logging") {
    ulog_output_level_set_all(ULOG_LEVEL_TRACE);

    ulog_trace("Trace message");
    ulog_debug("Debug message");
    ulog_info("Info message");
    ulog_warn("Warn message");
    ulog_error("Error message");
    ulog_fatal("Fatal message");

    CHECK(ut_callback_get_message_count() == 6);
    CHECK(strstr(ut_callback_get_last_message(), "Fatal message") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Static Config Header - Time Feature Enabled") {
    ulog_output_level_set_all(ULOG_LEVEL_INFO);

    ulog_info("Test message with time");

    // Check that time is present in the output (format: HH:MM:SS)
    const char *last_msg = ut_callback_get_last_message();
    CHECK(last_msg != nullptr);
    // Time format check - looking for pattern like "12:34:56"
    bool has_time = (strchr(last_msg, ':') != nullptr);
    CHECK(has_time);
}

TEST_CASE_FIXTURE(TestFixture,
                  "Static Config Header - Topics Feature Enabled") {
    ulog_output_level_set_all(ULOG_LEVEL_TRACE);

    // Add a topic
    ulog_topic_id test_topic = ulog_topic_add("TEST", ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);
    CHECK(test_topic != ULOG_TOPIC_ID_INVALID);

    ulog_topic_info("TEST", "Message with topic");

    CHECK(ut_callback_get_message_count() == 1);
    const char *last_msg = ut_callback_get_last_message();
    CHECK(strstr(last_msg, "[TEST]") != nullptr);
    CHECK(strstr(last_msg, "Message with topic") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Static Config Header - Extra Outputs Enabled") {
    // Create a file output
    const char *filename = "static_config_test.log";
    FILE *fp             = fopen(filename, "w");
    REQUIRE(fp != nullptr);

    ulog_output_id file_output = ulog_output_add_file(fp, ULOG_LEVEL_INFO);
    CHECK(file_output != ULOG_OUTPUT_INVALID);

    ulog_info("Message to file");

    // Remove the output handler before closing the file
    ulog_output_remove(file_output);
    fclose(fp);

    // Verify file was written
    fp = fopen(filename, "r");
    REQUIRE(fp != nullptr);

    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    CHECK(strstr(buffer, "Message to file") != nullptr);

    // Cleanup
    remove(filename);
}

TEST_CASE_FIXTURE(TestFixture,
                  "Static Config Header - Prefix Feature Enabled") {
    ulog_output_level_set_all(ULOG_LEVEL_INFO);

    ulog_status status = ulog_prefix_set_fn(test_prefix_handler);
    CHECK(status == ULOG_STATUS_OK);

    ulog_info("Test with prefix");

    const char *last_msg = ut_callback_get_last_message();
    CHECK(strstr(last_msg, "[PREFIX]") != nullptr);
    CHECK(strstr(last_msg, "Test with prefix") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture,
                  "Static Config Header - Source Location Enabled") {
    ulog_output_level_set_all(ULOG_LEVEL_INFO);

    ulog_info("Message with source location");

    const char *last_msg = ut_callback_get_last_message();
    // Should contain file:line format
    CHECK(strstr(last_msg, ".cpp:") != nullptr);
    CHECK(strstr(last_msg, "Message with source location") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Static Config Header - Level Filtering") {
    ulog_output_level_set_all(ULOG_LEVEL_WARN);

    ulog_trace("Should be filtered");
    ulog_debug("Should be filtered");
    ulog_info("Should be filtered");
    CHECK(ut_callback_get_message_count() == 0);

    ulog_warn("Should appear");
    ulog_error("Should appear");
    ulog_fatal("Should appear");
    CHECK(ut_callback_get_message_count() == 3);
}

TEST_CASE_FIXTURE(TestFixture,
                  "Static Config Header - Multiple Features Combined") {
    ulog_output_level_set_all(ULOG_LEVEL_INFO);

    // Add topic
    ulog_topic_id topic = ulog_topic_add("COMBINED", ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);
    CHECK(topic != ULOG_TOPIC_ID_INVALID);

    // Set prefix
    ulog_prefix_set_fn(combined_prefix_handler);

    ulog_topic_info("COMBINED", "All features active");

    const char *last_msg = ut_callback_get_last_message();
    // Should have time, prefix, topic, level, source location, and message
    CHECK(last_msg != nullptr);
    CHECK(strstr(last_msg, "[TEST]") != nullptr);
    CHECK(strstr(last_msg, "[COMBINED]") != nullptr);
    CHECK(strstr(last_msg, "All features active") != nullptr);
}
