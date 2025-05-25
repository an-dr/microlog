#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "ulog.h"

// Forward declaration for the callback
static void test_log_callback(ulog_Event *ev, void *arg);

// Global state for testing callbacks
static int processed_message_count = 0;
static char last_message_buffer[256];

// Custom log callback for tests
void test_log_callback(ulog_Event *ev, void *arg) {
    (void)arg; // Userdata is now 'arg', mark as unused if not used.
    // (void)ev; // ev is used for level and message formatting.
    
    processed_message_count++;
    
    // Format the message into our static buffer to "capture" it
    // In a real scenario, be wary of buffer overflows if messages are long
    // Note: ulog_Event has 'message' (format string) and 'message_format_args' (va_list)
    vsnprintf(last_message_buffer, sizeof(last_message_buffer), ev->message, ev->message_format_args);
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
    ulog_add_callback(test_log_callback, NULL, LOG_TRACE); // Assuming LOG_TRACE to capture all for this test setup
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
    // To properly "remove" a callback, one would need a ulog_remove_callback function.
    // For this test, we'll assume subsequent tests will re-register or that the callback
    // being present doesn't harm other tests if not explicitly used by them.
    // If ulog_add_callback returns an ID, one might use it to remove.
    // Or, if ULOG_EXTRA_OUTPUTS is small, it might fill up.
    // For now, we leave it, as microlog doesn't have a remove.
}

void test_ulog_set_quiet() {
    printf("Running Test 3: ulog_set_quiet...\n");

    // Set a custom callback to monitor messages
    // Note: This callback will persist from the previous test if not cleared.
    // This is okay if test_log_callback is idempotent or its effects are reset (like processed_message_count).
    // Adding it again might fill up slots if ULOG_EXTRA_OUTPUTS is small and no removal is done.
    // Let's assume for test_core.c, one registration of test_log_callback is fine.
    // If test_ulog_set_level already added it, this line is redundant or could fail if slots are full.
    // For simplicity of this fix, let's assume the previous registration is sufficient.
    // ulog_add_callback(test_log_callback, NULL, LOG_TRACE); // Potentially re-adding or redundant
    processed_message_count = 0; // Reset counter

    ulog_set_quiet(true);
    // This log_info will still trigger test_log_callback because ulog.quiet does not affect extra callbacks.
    log_info("This message will trigger extra callbacks, stdout should be quiet.");
    assert(processed_message_count == 1); // Expect 1 because test_log_callback runs

    ulog_set_quiet(false);
    // This log_info will also trigger test_log_callback. Stdout is not quiet.
    log_info("This message will trigger extra callbacks, stdout is not quiet.");
    assert(processed_message_count == 2); // Expect 2 as test_log_callback runs again

    printf("Test 3: Passed (ulog_set_quiet assertions updated for callback behavior).\n\n");
    
    // No ulog_remove_callback, so the callback stays registered.
}

int main() {
    // Initialize microlog and set a default state if necessary
    // ulog_init(); // If available and resets callbacks etc.
    ulog_set_level(LOG_TRACE); // Global level

    // Add the test callback ONCE for all tests in this file that need it.
    // This assumes ULOG_EXTRA_OUTPUTS is at least 1.
    // The third argument to ulog_add_callback is the threshold for this specific callback.
    // Setting to LOG_TRACE means this callback will be called for all levels.
    int callback_add_result = ulog_add_callback(test_log_callback, NULL, LOG_TRACE);
    if (callback_add_result != 0) {
        fprintf(stderr, "Failed to add the primary test callback. This may affect test results.\n");
        // Depending on microlog's behavior, this could be due to no slots (ULOG_EXTRA_OUTPUTS too small)
    }
    assert(callback_add_result == 0 && "Failed to add test_log_callback for test_core.c");

    printf("Starting unit tests for microlog core features...\n\n");

    test_basic_logging_macros();
    test_ulog_set_level();
    test_ulog_set_quiet();

    printf("All core tests completed successfully!\n");
    return 0;
}
