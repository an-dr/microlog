#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include <stdio.h>
#include "ulog.h"

// Test the ULOG_BUILD_DISABLED feature
// When ULOG_BUILD_DISABLED=1, all functions should return disabled status or stubs
// This test verifies that the macros work correctly when logging is disabled

struct TestFixture {
  public:
    TestFixture() {
        // No setup needed for disabled testing
    }

    ~TestFixture() = default;
};

// Test basic logging macros
TEST_CASE_FIXTURE(TestFixture, "Disabled - Basic Logging Macros") {
    // These should all be no-ops when disabled
    // They should compile to ((void)0) so arguments are NOT evaluated
    int counter = 0;
    
    ulog_trace("This is a TRACE message: %d", ++counter);
    ulog_debug("This is a DEBUG message: %s", ++counter ? "test" : "fail");
    ulog_info("This is an INFO message: %.2f", counter += 2);
    ulog_warn("This is a WARN message %d", ++counter);
    ulog_error("This is an ERROR message: %x", ++counter);
    ulog_fatal("This is a FATAL message %d", ++counter);
    
    // Generic macro
    ulog(ULOG_LEVEL_INFO, "Generic log message %d", ++counter);
    
    // Since the macros are ((void)0), arguments should NOT be evaluated
    // This is the intended behavior for zero-overhead when disabled
    CHECK(counter == 0);
}

// Test topic-based logging macros
TEST_CASE_FIXTURE(TestFixture, "Disabled - Topic Logging Macros") {
    // These should all be no-ops when disabled
    int counter = 0;
    
    ulog_topic_trace("network", "Network trace: %d", ++counter);
    ulog_topic_debug("storage", "Storage debug: %s", ++counter ? "test" : "fail");
    ulog_topic_info("ui", "UI info: %.2f", counter += 2);
    ulog_topic_warn("memory", "Memory warn %d", ++counter);
    ulog_topic_error("cpu", "CPU error: %x", ++counter);
    ulog_topic_fatal("system", "System fatal %d", ++counter);
    
    // Short aliases
    ulog_t_trace("net", "Net trace %d", ++counter);
    ulog_t_debug("store", "Store debug %d", ++counter);
    ulog_t_info("interface", "Interface info %d", ++counter);
    ulog_t_warn("mem", "Memory warn %d", ++counter);
    ulog_t_error("proc", "Processor error %d", ++counter);
    ulog_t_fatal("sys", "System fatal %d", ++counter);
    
    // Generic topic macro
    ulog_topic_log(ULOG_LEVEL_INFO, "general", "Generic topic message %d", ++counter);
    ulog_t(ULOG_LEVEL_WARN, "test", "Short topic macro %d", ++counter);
    
    // Since the macros are ((void)0), arguments should NOT be evaluated
    // This is the intended behavior for zero-overhead when disabled
    CHECK(counter == 0);
}

// Test status-returning functions
TEST_CASE_FIXTURE(TestFixture, "Disabled - Status Functions") {
    CHECK(ulog_cleanup() == ULOG_STATUS_DISABLED);
    CHECK(ulog_color_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_prefix_config(false) == ULOG_STATUS_DISABLED);
    CHECK(ulog_source_location_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_time_config(false) == ULOG_STATUS_DISABLED);
    CHECK(ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_DEFAULT) == ULOG_STATUS_DISABLED);
    CHECK(ulog_topic_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_level_reset_levels() == ULOG_STATUS_DISABLED);
    CHECK(ulog_level_set_new_levels(nullptr) == ULOG_STATUS_DISABLED);
    CHECK(ulog_lock_set_fn(nullptr, nullptr) == ULOG_STATUS_DISABLED);
    CHECK(ulog_prefix_set_fn(nullptr) == ULOG_STATUS_DISABLED);
    CHECK(ulog_output_level_set(ULOG_OUTPUT_STDOUT, ULOG_LEVEL_INFO) == ULOG_STATUS_DISABLED);
    CHECK(ulog_output_level_set_all(ULOG_LEVEL_WARN) == ULOG_STATUS_DISABLED);
    CHECK(ulog_output_remove(ULOG_OUTPUT_STDOUT) == ULOG_STATUS_DISABLED);
    CHECK(ulog_topic_level_set("test", ULOG_LEVEL_DEBUG) == ULOG_STATUS_DISABLED);
    CHECK(ulog_topic_remove("test") == ULOG_STATUS_DISABLED);
}

// Test event functions
TEST_CASE_FIXTURE(TestFixture, "Disabled - Event Functions") {
    // These should return appropriate disabled values
    CHECK(ulog_event_to_cstr(nullptr, nullptr, 0) == ULOG_STATUS_DISABLED);
    CHECK(ulog_event_get_message(nullptr, nullptr, 0) == ULOG_STATUS_DISABLED);
    CHECK(ulog_event_get_topic(nullptr) == ULOG_TOPIC_ID_INVALID);
    CHECK(ulog_event_get_line(nullptr) == -1);
    CHECK(ulog_event_get_file(nullptr) != nullptr);
    CHECK(strcmp(ulog_event_get_file(nullptr), "") == 0);
    CHECK(ulog_event_get_level(nullptr) == ULOG_LEVEL_0);
    CHECK(ulog_event_get_time(nullptr) == NULL);  // Use NULL instead of nullptr for C compatibility
}

// Test output functions
TEST_CASE_FIXTURE(TestFixture, "Disabled - Output Functions") {
    CHECK(ulog_output_add(nullptr, nullptr, ULOG_LEVEL_INFO) == ULOG_OUTPUT_INVALID);
    CHECK(ulog_output_add_file(nullptr, ULOG_LEVEL_DEBUG) == ULOG_OUTPUT_INVALID);
}

// Test topic functions
TEST_CASE_FIXTURE(TestFixture, "Disabled - Topic Functions") {
    CHECK(ulog_topic_add("test_topic", ULOG_OUTPUT_STDOUT, ULOG_LEVEL_INFO) == ULOG_TOPIC_ID_INVALID);
    CHECK(ulog_topic_get_id("test_topic") == ULOG_TOPIC_ID_INVALID);
}

// Test level functions
TEST_CASE_FIXTURE(TestFixture, "Disabled - Level Functions") {
    CHECK(ulog_level_to_string(ULOG_LEVEL_INFO) != nullptr);
    CHECK(strcmp(ulog_level_to_string(ULOG_LEVEL_INFO), "?") == 0);
    CHECK(strcmp(ulog_level_to_string(ULOG_LEVEL_ERROR), "?") == 0);
    CHECK(strcmp(ulog_level_to_string(ULOG_LEVEL_FATAL), "?") == 0);
}

// Test that macros don't produce side effects when disabled
TEST_CASE_FIXTURE(TestFixture, "Disabled - No Side Effects") {
    int counter = 0;
    
    // These should not increment the counter since logging is disabled
    ulog_info("Counter: %d", ++counter);
    ulog_debug("Counter: %d", ++counter); 
    ulog_topic_warn("test", "Counter: %d", ++counter);
    
    // Since the disabled macros are ((void)0), arguments are NOT evaluated
    // This provides zero-overhead when logging is disabled
    CHECK(counter == 0);  // No side effects should occur
}

// Test that macro expansion works correctly
TEST_CASE_FIXTURE(TestFixture, "Disabled - Macro Expansion") {
    // Verify that the macros are properly defined and don't cause compilation errors
    
    // These should all compile to ((void)0) and do nothing
    ulog_trace("test");
    ulog_debug("test");
    ulog_info("test");  
    ulog_warn("test");
    ulog_error("test");
    ulog_fatal("test");
    
    // Topic macros should also work
    ulog_t_trace("topic", "test");
    ulog_t_debug("topic", "test");
    ulog_t_info("topic", "test");
    ulog_t_warn("topic", "test");
    ulog_t_error("topic", "test");
    ulog_t_fatal("topic", "test");
    
    // If we reach this point, all macros compiled successfully
    CHECK(true);
}

// Test disabled behavior with file operations
TEST_CASE_FIXTURE(TestFixture, "Disabled - File Operations") {
    // These operations should return invalid/disabled status
    CHECK(ulog_output_add_file(NULL, ULOG_LEVEL_INFO) == ULOG_OUTPUT_INVALID);
    
    // Try logging - should be no-op
    ulog_info("This should not be written anywhere");
    
    // Check that we can call the function without crashes
    CHECK(true);  // If we get here, no crashes occurred
}

// Test comprehensive disabled state
TEST_CASE_FIXTURE(TestFixture, "Disabled - Comprehensive State") {
    // Test all combinations of function calls that should be disabled
    
    // Setup calls - all should return disabled status
    CHECK(ulog_lock_set_fn(NULL, NULL) == ULOG_STATUS_DISABLED);
    CHECK(ulog_prefix_set_fn(NULL) == ULOG_STATUS_DISABLED);
    
    // Configuration calls - all should return disabled status
    CHECK(ulog_color_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_time_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_source_location_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_prefix_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_topic_config(true) == ULOG_STATUS_DISABLED);
    CHECK(ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_SHORT) == ULOG_STATUS_DISABLED);
    
    // Output management - all should return invalid/disabled
    CHECK(ulog_output_add(NULL, NULL, ULOG_LEVEL_TRACE) == ULOG_OUTPUT_INVALID);
    CHECK(ulog_output_level_set(ULOG_OUTPUT_STDOUT, ULOG_LEVEL_FATAL) == ULOG_STATUS_DISABLED);
    CHECK(ulog_output_level_set_all(ULOG_LEVEL_ERROR) == ULOG_STATUS_DISABLED);
    CHECK(ulog_output_remove(42) == ULOG_STATUS_DISABLED);
    
    // Topic management - all should return invalid/disabled
    CHECK(ulog_topic_add("disabled_topic", ULOG_OUTPUT_ALL, ULOG_LEVEL_DEBUG) == ULOG_TOPIC_ID_INVALID);
    CHECK(ulog_topic_get_id("nonexistent") == ULOG_TOPIC_ID_INVALID);
    CHECK(ulog_topic_level_set("disabled_topic", ULOG_LEVEL_WARN) == ULOG_STATUS_DISABLED);
    CHECK(ulog_topic_remove("disabled_topic") == ULOG_STATUS_DISABLED);
    
    // Level management - all should return disabled
    CHECK(ulog_level_set_new_levels(NULL) == ULOG_STATUS_DISABLED);
    CHECK(ulog_level_reset_levels() == ULOG_STATUS_DISABLED);
    
    // Event handling - all should return appropriate disabled values
    char buffer[100];
    CHECK(ulog_event_to_cstr(NULL, buffer, sizeof(buffer)) == ULOG_STATUS_DISABLED);
    CHECK(ulog_event_get_message(NULL, buffer, sizeof(buffer)) == ULOG_STATUS_DISABLED);
    
    // Cleanup should be disabled
    CHECK(ulog_cleanup() == ULOG_STATUS_DISABLED);
    
    // Verify no logging occurred during all these calls
    CHECK(true);  // If we get here, all tests passed
}
