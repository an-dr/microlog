#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "ulog.h"

#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0

// Global state for topic logging callback
static int g_topic_log_count = 0;

// Callback to check if a log message for a specific topic was processed
static void topic_log_check_callback(ulog_Event *ev, void *arg) {
    (void)ev; // Event data not strictly needed for this test's logic
    (void)arg; // User argument not used in this simple callback
    g_topic_log_count++;
}

// Helper to reset state before each test group
void reset_topic_test_state() {
    g_topic_log_count = 0;
    // ulog_init(); // Ideal if ulog_init() clears all topics and callbacks.
    // If not, manual cleanup or relying on distinct topic names per test is needed.
    // For now, we'll rely on distinct topic names and specific enable/disable calls.
    // Also, clear extra outputs to avoid interference if ulog_init doesn't.
    // This assumes a function like ulog_clear_outputs() or that tests are sequential.
    // For simplicity, we'll add the callback once at the beginning of main.
}

void test_basic_topic_logging_and_filtering() {
    printf("Running Test 1: Basic Topic Logging & Filtering...\n");
    reset_topic_test_state();

    ulog_topic_id_t id1 = ulog_add_topic("TEST_TOPIC1", true);
    assert(id1 >= 0 && "Failed to add TEST_TOPIC1");

    ulog_topic_id_t retrieved_id1 = ulog_get_topic_id("TEST_TOPIC1");
    assert(retrieved_id1 == id1 && "Retrieved ID for TEST_TOPIC1 does not match");

    logt_info("TEST_TOPIC1", "Hello from TEST_TOPIC1");
    assert(g_topic_log_count == 1 && "Log count incorrect after logt_info to TEST_TOPIC1");

    log_info("This is a global log, should also be captured by the callback.");
    assert(g_topic_log_count == 2 && "Log count incorrect after global log_info");

    printf("Test 1: Passed.\n\n");
}

void test_disabling_enabling_topics() {
    printf("Running Test 2: Disabling/Enabling Topics...\n");
    reset_topic_test_state();

    ulog_topic_id_t id2 = ulog_add_topic("TEST_TOPIC2", true);
    assert(id2 >= 0 && "Failed to add TEST_TOPIC2");

    int disable_result = ulog_disable_topic("TEST_TOPIC2");
    assert(disable_result == 0 && "ulog_disable_topic for TEST_TOPIC2 failed");

    logt_info("TEST_TOPIC2", "This should NOT log (TEST_TOPIC2 disabled)");
    assert(g_topic_log_count == 0 && "Log count should be 0 after logging to disabled TEST_TOPIC2");

    int enable_result = ulog_enable_topic("TEST_TOPIC2");
    assert(enable_result == 0 && "ulog_enable_topic for TEST_TOPIC2 failed");

    logt_info("TEST_TOPIC2", "This SHOULD log (TEST_TOPIC2 re-enabled)");
    assert(g_topic_log_count == 1 && "Log count incorrect after logging to re-enabled TEST_TOPIC2");

    printf("Test 2: Passed.\n\n");
}

void test_topic_specific_log_levels() {
    printf("Running Test 3: Topic-Specific Log Levels...\n");
    reset_topic_test_state();
    // Assuming global log level is TRACE or DEBUG for this test to pass correctly for global logs.
    // ulog_set_level(LOG_TRACE); 

    ulog_topic_id_t id3 = ulog_add_topic("TEST_TOPIC3", true);
    assert(id3 >= 0 && "Failed to add TEST_TOPIC3");

    int set_level_result = ulog_set_topic_level("TEST_TOPIC3", LOG_WARN);
    assert(set_level_result == 0 && "ulog_set_topic_level for TEST_TOPIC3 failed");

    logt_info("TEST_TOPIC3", "Info for TEST_TOPIC3 - should NOT log");
    assert(g_topic_log_count == 0 && "Log count should be 0 after logt_info to TEST_TOPIC3 (level WARN)");

    logt_warn("TEST_TOPIC3", "Warn for TEST_TOPIC3 - SHOULD log");
    assert(g_topic_log_count == 1 && "Log count incorrect after logt_warn to TEST_TOPIC3");

    // Assuming the callback also captures non-topic logs
    log_warn("Global warn - should also log via callback");
    assert(g_topic_log_count == 2 && "Log count incorrect after global log_warn");

    // Reset topic level to default (e.g. global level or enabled) for subsequent tests if any
    // ulog_set_topic_level("TEST_TOPIC3", ulog_get_level()); // Or a sensible default
    printf("Test 3: Passed.\n\n");
}

void test_enable_disable_all_topics() {
    printf("Running Test 4: ulog_enable_all_topics / ulog_disable_all_topics...\n");
    reset_topic_test_state();

    ulog_topic_id_t id_all1 = ulog_add_topic("T_ALL1", true); // Start enabled
    ulog_topic_id_t id_all2 = ulog_add_topic("T_ALL2", true); // Start enabled
    assert(id_all1 >= 0 && "Failed to add T_ALL1");
    assert(id_all2 >= 0 && "Failed to add T_ALL2");

    ulog_disable_all_topics();
    printf("Disabled all topics.\n");

    logt_info("T_ALL1", "Log to T_ALL1 (should be disabled)");
    assert(g_topic_log_count == 0 && "Log count non-zero after T_ALL1 log (all disabled)");
    logt_info("T_ALL2", "Log to T_ALL2 (should be disabled)");
    assert(g_topic_log_count == 0 && "Log count non-zero after T_ALL2 log (all disabled)");

    ulog_enable_all_topics(true); // true = new topics also enabled by default
    printf("Enabled all topics.\n");

    logt_info("T_ALL1", "Log to T_ALL1 (should be re-enabled)");
    assert(g_topic_log_count == 1 && "Log count incorrect for T_ALL1 (all enabled)");
    logt_info("T_ALL2", "Log to T_ALL2 (should be re-enabled)");
    assert(g_topic_log_count == 2 && "Log count incorrect for T_ALL2 (all enabled)");
    
    printf("Test 4: Passed.\n\n");
}

void test_non_existent_topic() {
    printf("Running Test 5: Non-existent topic logging...\n");
    reset_topic_test_state();
    const char* non_existent_topic_name = "NON_EXISTENT_TOPIC";

    // Ensure the topic does not exist before logging to it for a clean test
    // (especially important if tests run multiple times or topics persist)
    // If ulog_remove_topic exists, use it. Otherwise, this test depends on the topic truly not being there.
    // ulog_topic_id_t pre_check_id = ulog_get_topic_id(non_existent_topic_name);
    // if (pre_check_id != -1) { ulog_remove_topic(non_existent_topic_name); }


    logt_info(non_existent_topic_name, "Logging to a topic that might not exist.");

#if ULOG_TOPICS_NUM == -1 // Dynamic allocation
    printf("Dynamic topic allocation (ULOG_TOPICS_NUM == -1)\n");
    // In dynamic mode, logging to a non-existent topic might add it.
    // The default enabled state of such auto-added topics depends on ulog_get_new_topic_enabled_by_default()
    // or the parameter to ulog_enable_all_topics(). Assume it's enabled for this test.
    ulog_topic_id_t id_ne = ulog_get_topic_id(non_existent_topic_name);
    assert(id_ne >= 0 && "Non-existent topic was not dynamically added or ID not found.");
    assert(g_topic_log_count == 1 && "Log count incorrect for dynamically added topic.");
    printf("Non-existent topic was dynamically added and logged.\n");
#else // Static allocation (ULOG_TOPICS_NUM > 0)
    printf("Static topic allocation (ULOG_TOPICS_NUM > 0)\n");
    // In static mode, logging to a non-existent topic should not work if it wasn't pre-added.
    ulog_topic_id_t id_ne_static = ulog_get_topic_id(non_existent_topic_name);
    assert(id_ne_static == -1 && "Topic should not exist in static mode unless pre-added.");
    assert(g_topic_log_count == 0 && "Log count should be 0 for non-existent topic in static mode.");
    printf("Non-existent topic was (correctly) not logged in static mode.\n");
#endif

    printf("Test 5: Passed.\n\n");
}


int main() {
    printf("Starting unit tests for microlog topics...\n\n");

    // Initialize microlog (if needed, and if it resets state)
    // ulog_init(); 
    ulog_set_level(LOG_TRACE); // Ensure global level allows all messages for callback checks

    // Add the callback once for all tests.
    // Ensure ULOG_EXTRA_OUTPUTS is defined and >0 for this to work.
    // This test suite assumes ULOG_EXTRA_OUTPUTS >= 1.
    // If not, the callback won't be added, and g_topic_log_count will never increment.
    // The CMake setup for these tests should ensure ULOG_EXTRA_OUTPUTS is set.
    int cb_add_result = ulog_add_callback(topic_log_check_callback, NULL, LOG_TRACE);
    if (cb_add_result != 0) {
        printf("CRITICAL: Failed to add topic_log_check_callback. Tests will not be meaningful.\n");
        // This could happen if ULOG_EXTRA_OUTPUTS is 0 or full.
        // For these tests, we'll assert it was added, implying ULOG_EXTRA_OUTPUTS is correctly set.
    }
    assert(cb_add_result == 0 && "Failed to add the global topic log check callback.");


    test_basic_topic_logging_and_filtering();
    test_disabling_enabling_topics();
    test_topic_specific_log_levels();
    test_enable_disable_all_topics();
    test_non_existent_topic();

    printf("All topic tests completed!\n");
    return 0;
}

#else
int main() {
    printf("ULOG_TOPICS_NUM is not defined or is 0. Skipping topic tests.\n");
    assert(1 == 1 && "Skipping test as ULOG_TOPICS_NUM is not enabled/set.");
    return 0;
}
#endif // ULOG_TOPICS_NUM && ULOG_TOPICS_NUM != 0
