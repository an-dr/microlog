#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ulog.h"
#include "ut_callback.h"
#include "ut_test_suite.h"

void test_basic_logging_macros() {
    log_trace("This is a TRACE message: %d", 123);
    log_debug("This is a DEBUG message: %s", "test");
    log_info("This is an INFO message: %.2f", 1.23);
    log_warn("This is a WARN message");
    log_error("This is an ERROR message: %x", 0xff);
    log_fatal("This is a FATAL message");
    assert(ut_callback_get_message_count() == 6);
    assert(strcmp(ut_callback_get_last_message(), "This is a FATAL message") ==
           0);
}

void test_ulog_set_level() {
    ulog_set_level(LOG_INFO);

    log_trace("This TRACE should not be processed.");
    assert(ut_callback_get_message_count() == 0);
    log_debug("This DEBUG should not be processed.");
    assert(ut_callback_get_message_count() == 0);
    log_info("This INFO should be processed.");
    assert(ut_callback_get_message_count() == 1);
    log_warn("This WARN should be processed.");
    assert(ut_callback_get_message_count() == 2);
    log_error("This ERROR should be processed.");
    assert(ut_callback_get_message_count() == 3);
    log_fatal("This FATAL should be processed.");
    assert(ut_callback_get_message_count() == 4);
}

void test_ulog_set_quiet() {
    ulog_set_quiet(true);
    // This log_info will still trigger test_log_callback because ulog.quiet
    // does not affect extra callbacks.
    log_info(
        "This message will trigger extra callbacks, stdout should be quiet.");
    assert(ut_callback_get_message_count() ==
           1);  // Expect 1 because test_log_callback runs

    ulog_set_quiet(false);
    // This log_info will also trigger test_log_callback. Stdout is not quiet.
    log_info("This message will trigger extra callbacks, stdout is not quiet.");
    assert(ut_callback_get_message_count() ==
           2);  // Expect 2 as test_log_callback runs again

    printf("Test: %s - Passed\n\n", __func__);

    // No ulog_remove_callback, so the callback stays registered.
}

void setup() {
    printf("Running setup...\n");
    // This function can be used to set up the environment before tests.
    // For example, you might want to initialize the logger or reset states.
    ulog_set_level(LOG_TRACE);  // Set the default log level to TRACE
    ulog_set_quiet(false);      // Ensure quiet mode is off for tests
    ut_callback_reset();        // Reset the callback state
    printf("Setup complete.\n");
}

void setup_suite() {
    printf("Running setup suite...\n");
    int callback_add_result = ulog_add_callback(ut_callback, NULL, LOG_TRACE);
    if (callback_add_result != 0) {
        fprintf(stderr, "Failed to add the primary test callback. This may "
                        "affect test results.\n");
        // Depending on microlog's behavior, this could be due to no slots
        // (ULOG_EXTRA_OUTPUTS too small)
    }
    assert(callback_add_result == 0 &&
           "Failed to add test_log_callback for test_core.c");
    printf("Setup suite complete.\n");
}

int main() {

    TestSuite suite;
    TestSuite_init(&suite, setup_suite, NULL, setup, NULL);

    printf("Starting unit tests for microlog core features...\n\n");

    TestSuite_add_test(&suite, "Macros", test_basic_logging_macros);
    TestSuite_add_test(&suite, "Levels", test_ulog_set_level);
    TestSuite_add_test(&suite, "Quiet Mode", test_ulog_set_quiet);
    TestSuite_run(&suite);

    printf("All core tests completed successfully!\n");
    return 0;
}
