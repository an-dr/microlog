#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include <stdio.h>
#include "ulog.h"
#include "ut_callback.h"

// Test callback for secondary output
static int secondary_message_count = 0;
static char secondary_last_message[256] = {0};

void secondary_callback(ulog_event *ev, void *arg) {
    (void)arg;  // Unused
    
    if (!ev) {
        secondary_last_message[0] = '\0';
        return;
    }
    
    memset(secondary_last_message, 0, sizeof(secondary_last_message));
    ulog_event_to_cstr(ev, secondary_last_message, sizeof(secondary_last_message));
    secondary_message_count++;
}

void secondary_callback_reset() {
    secondary_message_count = 0;
    secondary_last_message[0] = '\0';
}

int secondary_callback_get_count() {
    return secondary_message_count;
}

char* secondary_callback_get_last_message() {
    return secondary_last_message;
}

struct OutputTestFixture {
  public:
    static bool callbacks_set;
    static ulog_output_id secondary_output_id;

    OutputTestFixture() {
        // Per-test setup
        if (!callbacks_set) {
            // Add the main test callback (this becomes output ID 1)
            ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
            
            // Add secondary callback (this becomes output ID 2)
            secondary_output_id = ulog_output_add(secondary_callback, nullptr, ULOG_LEVEL_TRACE);
            callbacks_set = true;
        }
        
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        ut_callback_reset();
        secondary_callback_reset();
    }

    ~OutputTestFixture() = default;
};

bool OutputTestFixture::callbacks_set = false;
ulog_output_id OutputTestFixture::secondary_output_id = ULOG_OUTPUT_INVALID;

TEST_CASE_FIXTURE(OutputTestFixture, "Log to all outputs") {
    // Test that ULOG_OUTPUT_ALL logs to all outputs
    ulog_output_info(ULOG_OUTPUT_ALL, "Test message to all outputs");
    
    // Both callbacks should receive the message
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(secondary_callback_get_count() == 1);
    
    CHECK(strstr(ut_callback_get_last_message(), "Test message to all outputs") != nullptr);
    CHECK(strstr(secondary_callback_get_last_message(), "Test message to all outputs") != nullptr);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Log to stdout only") {
    // Test that ULOG_OUTPUT_STDOUT only logs to stdout (not to our test callbacks)
    ulog_output_info(ULOG_OUTPUT_STDOUT, "Test message to stdout only");
    
    // Neither test callback should receive the message (it goes to stdout)
    CHECK(ut_callback_get_message_count() == 0);
    CHECK(secondary_callback_get_count() == 0);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Log to specific output") {
    // Test logging to the secondary callback only
    ulog_output_info(secondary_output_id, "Test message to secondary output");
    
    // Only the secondary callback should receive the message
    CHECK(ut_callback_get_message_count() == 0);
    CHECK(secondary_callback_get_count() == 1);
    
    CHECK(strstr(secondary_callback_get_last_message(), "Test message to secondary output") != nullptr);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Log to invalid output") {
    // Test logging to an invalid output ID
    ulog_output_info(ULOG_OUTPUT_INVALID, "Test message to invalid output");
    ulog_output_info(999, "Test message to non-existent output");
    
    // No callbacks should receive the messages
    CHECK(ut_callback_get_message_count() == 0);
    CHECK(secondary_callback_get_count() == 0);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Output level filtering") {
    // Set secondary output to only accept WARN and above
    ulog_output_level_set(secondary_output_id, ULOG_LEVEL_WARN);
    
    // Test different levels
    ulog_output_trace(secondary_output_id, "Trace message");
    ulog_output_debug(secondary_output_id, "Debug message");
    ulog_output_info(secondary_output_id, "Info message");
    ulog_output_warn(secondary_output_id, "Warn message");
    ulog_output_error(secondary_output_id, "Error message");
    
    // Only WARN and ERROR should be received
    CHECK(secondary_callback_get_count() == 2);
    CHECK(strstr(secondary_callback_get_last_message(), "Error message") != nullptr);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Multiple messages to specific output") {
    // Test multiple messages to the same specific output
    ulog_output_info(secondary_output_id, "First message");
    ulog_output_warn(secondary_output_id, "Second message");
    ulog_output_error(secondary_output_id, "Third message");
    
    CHECK(secondary_callback_get_count() == 3);
    CHECK(ut_callback_get_message_count() == 0);  // Main callback should get nothing
    
    // Last message should be the third one
    CHECK(strstr(secondary_callback_get_last_message(), "Third message") != nullptr);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Different levels to different outputs") {
    // Log same level to different outputs
    ulog_output_info(ULOG_OUTPUT_ALL, "Message to all");
    ulog_output_info(secondary_output_id, "Message to secondary");
    
    // Both outputs should get the "all" message, only secondary gets the specific message
    CHECK(ut_callback_get_message_count() == 1);  // Only the "all" message
    CHECK(secondary_callback_get_count() == 2);   // Both messages
    
    CHECK(strstr(ut_callback_get_last_message(), "Message to all") != nullptr);
    CHECK(strstr(secondary_callback_get_last_message(), "Message to secondary") != nullptr);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Output with formatting") {
    // Test that formatting works correctly with specific outputs
    int value = 42;
    float pi = 3.14159f;
    
    ulog_output_info(secondary_output_id, "Integer: %d, Float: %.2f", value, pi);
    
    CHECK(secondary_callback_get_count() == 1);
    CHECK(ut_callback_get_message_count() == 0);
    
    char* last_msg = secondary_callback_get_last_message();
    CHECK(strstr(last_msg, "Integer: 42") != nullptr);
    CHECK(strstr(last_msg, "Float: 3.14") != nullptr);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Output removal") {
    // First verify the output works
    ulog_output_info(secondary_output_id, "Before removal");
    CHECK(secondary_callback_get_count() == 1);
    
    // Remove the output
    ulog_status status = ulog_output_remove(secondary_output_id);
    CHECK(status == ULOG_STATUS_OK);
    
    // Reset counter for clean test
    secondary_callback_reset();
    
    // Try to log to the removed output
    ulog_output_info(secondary_output_id, "After removal");
    
    // Should not receive the message
    CHECK(secondary_callback_get_count() == 0);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Cannot remove stdout output") {
    // Try to remove the stdout output - should fail
    ulog_status status = ulog_output_remove(ULOG_OUTPUT_STDOUT);
    CHECK(status == ULOG_STATUS_ERROR);
}

TEST_CASE_FIXTURE(OutputTestFixture, "Remove invalid output") {
    // Try to remove invalid output IDs
    ulog_status status1 = ulog_output_remove(ULOG_OUTPUT_INVALID);
    ulog_status status2 = ulog_output_remove(999);
    
    CHECK(status1 == ULOG_STATUS_INVALID_ARGUMENT);
    CHECK(status2 == ULOG_STATUS_INVALID_ARGUMENT);
}

#if ULOG_HAS_TOPICS
TEST_CASE_FIXTURE(OutputTestFixture, "Topic with specific output") {
    // Enable topic for testing
    ulog_topic_add("TestTopic", true);
    
    // Log with topic to specific output
    ulog_topic_output_info("TestTopic", secondary_output_id, "Topic message to specific output");
    
    CHECK(secondary_callback_get_count() == 1);
    CHECK(ut_callback_get_message_count() == 0);
    
    char* last_msg = secondary_callback_get_last_message();
    CHECK(strstr(last_msg, "[TestTopic]") != nullptr);
    CHECK(strstr(last_msg, "Topic message to specific output") != nullptr);
}
#endif
