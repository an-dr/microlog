//  unit tests for event getter functions
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

extern "C" {
#include "ulog.h"
#include "ut_callback.h"
}

#include <cstdio>
#include <cstring>
#include <time.h>

// Global variables to capture event data from callback
static ulog_level captured_level = ULOG_LEVEL_TRACE;
static char captured_message[512] = {0};
static char captured_full_string[512] = {0};
static bool event_captured = false;

#if ULOG_HAS_SOURCE_LOCATION
static char captured_file[256] = {0};
static int captured_line = -1;
#endif

#if ULOG_HAS_TIME
static struct tm captured_time = {0};
static bool captured_time_valid = false;
#endif

#if ULOG_HAS_TOPICS
static ulog_topic_id captured_topic_id = ULOG_TOPIC_ID_INVALID;
#endif

// Custom callback to capture event data (not the pointer itself)
void event_capture_callback(ulog_event *ev, void *arg) {
    (void)arg;
    
    if (ev == nullptr) {
        event_captured = false;
        return;
    }
    
    // Capture data from the event while it's valid
    captured_level = ulog_event_get_level(ev);
    
    ulog_event_get_message(ev, captured_message, sizeof(captured_message));
    ulog_event_to_cstr(ev, captured_full_string, sizeof(captured_full_string));
    
#if ULOG_HAS_SOURCE_LOCATION
    const char *file = ulog_event_get_file(ev);
    if (file) {
        strncpy(captured_file, file, sizeof(captured_file) - 1);
        captured_file[sizeof(captured_file) - 1] = '\0';
    } else {
        captured_file[0] = '\0';
    }
    captured_line = ulog_event_get_line(ev);
#endif

#if ULOG_HAS_TIME
    struct tm *time_ptr = ulog_event_get_time(ev);
    if (time_ptr) {
        captured_time = *time_ptr;
        captured_time_valid = true;
    } else {
        captured_time_valid = false;
    }
#endif

#if ULOG_HAS_TOPICS
    captured_topic_id = ulog_event_get_topic(ev);
#endif
    
    event_captured = true;
}

void reset_event_capture() {
    event_captured = false;
    captured_level = ULOG_LEVEL_TRACE;
    captured_message[0] = '\0';
    captured_full_string[0] = '\0';
    
#if ULOG_HAS_SOURCE_LOCATION
    captured_file[0] = '\0';
    captured_line = -1;
#endif

#if ULOG_HAS_TIME
    captured_time_valid = false;
#endif

#if ULOG_HAS_TOPICS
    captured_topic_id = ULOG_TOPIC_ID_INVALID;
#endif
}

// Test fixture for event getter tests
struct EventGettersTestFixture {
    EventGettersTestFixture() {
        ut_callback_reset();
        reset_event_capture();
        ulog_cleanup();
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        
        // Set up callback to capture events
        ulog_output_add(event_capture_callback, nullptr, ULOG_LEVEL_TRACE);
    }
    
    ~EventGettersTestFixture() {
        ulog_cleanup();
    }
    
    // Helper to trigger logging and capture an event
    void capture_event_from_log(ulog_level level, const char* message) {
        reset_event_capture();
        ulog_log(level, __FILE__, __LINE__, nullptr, "%s", message);
        REQUIRE(event_captured);
    }
    
    void capture_event_from_topic_log(ulog_level level, const char* topic, const char* message) {
        reset_event_capture();
        ulog_log(level, __FILE__, __LINE__, topic, "%s", message);
        REQUIRE(event_captured);
    }
};

TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Get Level") {
    // Test different log levels
    capture_event_from_log(ULOG_LEVEL_TRACE, "Test trace");
    CHECK(captured_level == ULOG_LEVEL_TRACE);
    
    capture_event_from_log(ULOG_LEVEL_DEBUG, "Test debug");
    CHECK(captured_level == ULOG_LEVEL_DEBUG);
    
    capture_event_from_log(ULOG_LEVEL_INFO, "Test info");
    CHECK(captured_level == ULOG_LEVEL_INFO);
    
    capture_event_from_log(ULOG_LEVEL_WARN, "Test warn");
    CHECK(captured_level == ULOG_LEVEL_WARN);
    
    capture_event_from_log(ULOG_LEVEL_ERROR, "Test error");
    CHECK(captured_level == ULOG_LEVEL_ERROR);
    
    capture_event_from_log(ULOG_LEVEL_FATAL, "Test fatal");
    CHECK(captured_level == ULOG_LEVEL_FATAL);
    
    // Test with NULL event
    ulog_level level = ulog_event_get_level(nullptr);
    CHECK(level == ULOG_LEVEL_TRACE); // Function should return TRACE for NULL
}

TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Get Message") {
    capture_event_from_log(ULOG_LEVEL_INFO, "Hello World");
    
    // The captured message includes file and line info
    CHECK(strstr(captured_message, "Hello World") != nullptr);
    CHECK(strstr(captured_message, "test_event_getters.cpp") != nullptr);
    
    // Test with formatted message
    reset_event_capture();
    ulog_info("Number: %d, String: %s", 42, "test");
    REQUIRE(event_captured);
    
    CHECK(strstr(captured_message, "Number: 42, String: test") != nullptr);
    CHECK(strstr(captured_message, "test_event_getters.cpp") != nullptr);
    
    // Test the actual ulog_event_get_message function with NULL parameters
    char buffer[256];
    ulog_status result = ulog_event_get_message(nullptr, buffer, sizeof(buffer));
    CHECK(result == ULOG_STATUS_INVALID_ARGUMENT);
}

TEST_CASE_FIXTURE(EventGettersTestFixture, "Event To CString") {
    capture_event_from_log(ULOG_LEVEL_INFO, "Complete log message");
    
    // Check that we have a valid captured string
    REQUIRE(strlen(captured_full_string) > 0);
    
    // The output should contain the message
    CHECK(strstr(captured_full_string, "Complete log message") != nullptr);
    // Should contain the level
    CHECK(strstr(captured_full_string, "INFO") != nullptr);
    // Should contain file information (if enabled)
    CHECK(strstr(captured_full_string, "test_event_getters.cpp") != nullptr);
}

#if ULOG_HAS_SOURCE_LOCATION
TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Get File and Line") {
    int test_line = __LINE__ + 1;
    capture_event_from_log(ULOG_LEVEL_INFO, "File test");
    
    // Test file name
    CHECK(strstr(captured_file, "test_event_getters.cpp") != nullptr);
    
    // Test line number
    CHECK(captured_line == test_line);
    
    // Test with NULL event
    const char *file = ulog_event_get_file(nullptr);
    CHECK(file == nullptr);
    
    int line = ulog_event_get_line(nullptr);
    CHECK(line == -1);
}
#endif // ULOG_HAS_SOURCE_LOCATION

#if ULOG_HAS_TIME
TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Get Time") {
    time_t before_log = time(nullptr);
    capture_event_from_log(ULOG_LEVEL_INFO, "Time test");
    time_t after_log = time(nullptr);
    
    CHECK(captured_time_valid);
    
    // Convert the tm structure back to time_t for comparison
    time_t event_time_t = mktime(&captured_time);
    
    // The event time should be between before_log and after_log
    CHECK(event_time_t >= before_log);
    CHECK(event_time_t <= after_log);
    
    // Test with NULL event
    struct tm *null_time = ulog_event_get_time(nullptr);
    CHECK(null_time == nullptr);
}
#endif // ULOG_HAS_TIME

#if ULOG_HAS_TOPICS
TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Get Topic") {
    // Add a topic for testing
    ulog_topic_id topic_id = ulog_topic_add("testgetter", ULOG_OUTPUT_ALL, true);
    REQUIRE(topic_id != ULOG_TOPIC_ID_INVALID);
    
    // Log with topic
    capture_event_from_topic_log(ULOG_LEVEL_INFO, "testgetter", "Topic test");
    
    CHECK(captured_topic_id == topic_id);
    
    // Log without topic
    capture_event_from_log(ULOG_LEVEL_INFO, "No topic test");
    
    CHECK(captured_topic_id == ULOG_TOPIC_ID_INVALID);
    
    // Test with NULL event
    ulog_topic_id event_topic = ulog_event_get_topic(nullptr);
    CHECK(event_topic == ULOG_TOPIC_ID_INVALID);
}
#endif // ULOG_HAS_TOPICS

TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Getters with Complex Log") {
    // Test all getters with a complex log entry
    
#if ULOG_HAS_TOPICS
    ulog_topic_id topic_id = ulog_topic_add("complex", ULOG_OUTPUT_ALL, true);
    REQUIRE(topic_id != ULOG_TOPIC_ID_INVALID);
#endif
    
    time_t before_log = time(nullptr);
    int test_line = __LINE__ + 4;
    
    reset_event_capture();
#if ULOG_HAS_TOPICS
    ulog_topic_warn("complex", "Complex message with number %d and string %s", 123, "abc");
#else
    ulog_warn("Complex message with number %d and string %s", 123, "abc");
#endif
    time_t after_log = time(nullptr);
    
    REQUIRE(event_captured);
    
    // Test level
    CHECK(captured_level == ULOG_LEVEL_WARN);
    
    // Test message
    CHECK(strstr(captured_message, "Complex message with number 123 and string abc") != nullptr);
    
#if ULOG_HAS_SOURCE_LOCATION
    // Test file
    CHECK(strstr(captured_file, "test_event_getters.cpp") != nullptr);
    
    // Test line
    CHECK(captured_line == test_line);
#endif
    
#if ULOG_HAS_TIME
    // Test time
    CHECK(captured_time_valid);
    time_t event_time_t = mktime(&captured_time);
    CHECK(event_time_t >= before_log);
    CHECK(event_time_t <= after_log);
#endif
    
#if ULOG_HAS_TOPICS
    // Test topic
    CHECK(captured_topic_id == topic_id);
#endif
    
    // Test complete string conversion
    CHECK(strstr(captured_full_string, "WARN") != nullptr);
    CHECK(strstr(captured_full_string, "Complex message with number 123 and string abc") != nullptr);
    
#if ULOG_HAS_TOPICS
    CHECK(strstr(captured_full_string, "[complex]") != nullptr);
#endif
    
#if ULOG_HAS_SOURCE_LOCATION
    CHECK(strstr(captured_full_string, "test_event_getters.cpp") != nullptr);
#endif
}

TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Getters Performance") {
    // Test that we can call getters multiple times without issues
    capture_event_from_log(ULOG_LEVEL_INFO, "Performance test");
    
    // Call getters multiple times on captured data
    for (int i = 0; i < 100; i++) {
        CHECK(captured_level == ULOG_LEVEL_INFO);
        CHECK(strstr(captured_message, "Performance test") != nullptr);
        
#if ULOG_HAS_SOURCE_LOCATION
        CHECK(strstr(captured_file, "test_event_getters.cpp") != nullptr);
        CHECK(captured_line > 0);
#endif
        
#if ULOG_HAS_TIME
        CHECK(captured_time_valid);
#endif
        
#if ULOG_HAS_TOPICS
        CHECK(captured_topic_id == ULOG_TOPIC_ID_INVALID); // No topic in this test
#endif
    }
}

TEST_CASE_FIXTURE(EventGettersTestFixture, "Event String Conversion Variations") {
    // Test different scenarios for ulog_event_to_cstr
    
    // Test with very long message
    capture_event_from_log(ULOG_LEVEL_ERROR, "This is a very long message that might test buffer handling and truncation behavior in the event to string conversion function. It contains multiple sentences and should demonstrate how the library handles longer content when converting events to string representation.");
    
    CHECK(strlen(captured_full_string) > 0);
    CHECK(strstr(captured_full_string, "ERROR") != nullptr);
    
    // Test with empty message
    capture_event_from_log(ULOG_LEVEL_DEBUG, "");
    
    // Should still contain level and other metadata
    CHECK(strstr(captured_full_string, "DEBUG") != nullptr);
}

TEST_CASE_FIXTURE(EventGettersTestFixture, "Event Getters Thread Safety Context") {
    // This test ensures that event getters work correctly within the callback context
    // where the logging lock is already held
    
    static bool callback_test_passed = false;
    static auto test_callback = [](ulog_event *ev, void *arg) -> void {
        (void)arg;
        
        if (!ev) return;
        
        // Test getters within callback
        ulog_level level = ulog_event_get_level(ev);
        
        char message_buffer[256];
        ulog_status result = ulog_event_get_message(ev, message_buffer, sizeof(message_buffer));
        
        char full_buffer[512];
        ulog_status full_result = ulog_event_to_cstr(ev, full_buffer, sizeof(full_buffer));
        
        // All operations should succeed
        callback_test_passed = (level == ULOG_LEVEL_INFO && 
                               result == ULOG_STATUS_OK && 
                               full_result == ULOG_STATUS_OK &&
                               strstr(message_buffer, "Callback test") != nullptr);
    };
    
    // Add our test callback
    ulog_output_add(test_callback, nullptr, ULOG_LEVEL_TRACE);
    
    callback_test_passed = false;
    ulog_info("Callback test message");
    
    CHECK(callback_test_passed);
}
