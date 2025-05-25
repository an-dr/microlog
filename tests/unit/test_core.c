#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "ulog.h"

// Global state for testing callbacks
static int processed_message_count = 0;
static char last_message_buffer[256];

// Custom log callback for tests
void test_log_callback(void *userdata, log_level_t level, const char *fmt, va_list args) {
    (void)userdata; // Unused
    (void)level;    // Unused for now, but could be used for more specific checks
    
    processed_message_count++;
    
    // Format the message into our static buffer to "capture" it
    // In a real scenario, be wary of buffer overflows if messages are long
    vsnprintf(last_message_buffer, sizeof(last_message_buffer), fmt, args);
}

void test_basic_logging_macros() {
    printf("Running Test 1: Basic Logging Macros...\n");
    log_trace("This is a TRACE message: %d", 123);
    log_debug("This is a DEBUG message: %s", "test");
    log_info("This is an INFO message: %.2f", 1.23);
    log_warn("This is a WARN message");
    log_error("This is an ERROR message: %x", 0xff);
    log_fatal("This is a FATAL message");
    printf("Test 1: Passed (Macros compiled and ran).\n\n");
}

void test_ulog_set_level() {
    printf("Running Test 2: ulog_set_level...\n");
    
    // Set a custom callback to monitor messages
    ulog_set_callback(test_log_callback, NULL);
    processed_message_count = 0; // Reset counter

    ulog_set_level(LOG_INFO);

    log_trace("This TRACE should not be processed.");
    log_debug("This DEBUG should not be processed.");
    log_info("This INFO should be processed.");
    log_warn("This WARN should be processed.");
    log_error("This ERROR should be processed.");
    log_fatal("This FATAL should be processed.");

    // We expect 4 messages (INFO, WARN, ERROR, FATAL)
    assert(processed_message_count == 4); 
    printf("Test 2: Passed (ulog_set_level correctly filtered messages).\n\n");
    
    // Reset to default level and callback for other tests
    ulog_set_level(LOG_TRACE); 
    ulog_set_callback(NULL, NULL); 
}

void test_ulog_set_quiet() {
    printf("Running Test 3: ulog_set_quiet...\n");

    // Set a custom callback to monitor messages
    ulog_set_callback(test_log_callback, NULL);
    processed_message_count = 0; // Reset counter

    ulog_set_quiet(true);
    log_info("This message should NOT be processed (quiet mode).");
    assert(processed_message_count == 0);

    ulog_set_quiet(false);
    log_info("This message SHOULD be processed (quiet mode off).");
    assert(processed_message_count == 1);

    printf("Test 3: Passed (ulog_set_quiet worked as expected).\n\n");
    
    // Reset callback for other tests
    ulog_set_callback(NULL, NULL);
}

int main() {
    printf("Starting unit tests for microlog core features...\n\n");

    test_basic_logging_macros();
    test_ulog_set_level();
    test_ulog_set_quiet();

    printf("All core tests completed successfully!\n");
    return 0;
}
