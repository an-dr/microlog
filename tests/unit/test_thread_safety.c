#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "ulog.h"

// Global struct to track lock/unlock calls and user data
typedef struct {
    bool locked;
    int lock_calls;
    int unlock_calls;
    void *expected_udata;
    void *actual_udata;
} lock_test_data_t;

static lock_test_data_t g_lock_data;

// Mock lock function
static void my_mock_lock_fn(bool lock, void *udata) {
    g_lock_data.actual_udata = udata;
    if (lock) {
        // Simulate behavior: ensure not already locked if our simple model is correct
        // assert(g_lock_data.locked == false && "Mock lock function: Lock called while already locked.");
        g_lock_data.locked = true;
        g_lock_data.lock_calls++;
    } else {
        // Simulate behavior: ensure was locked if our simple model is correct
        // assert(g_lock_data.locked == true && "Mock lock function: Unlock called while not locked.");
        g_lock_data.locked = false;
        g_lock_data.unlock_calls++;
    }
}

void test_ulog_set_lock_functionality() {
    printf("Running test_ulog_set_lock_functionality...\n");

    // Initialize g_lock_data
    g_lock_data.locked = false;
    g_lock_data.lock_calls = 0;
    g_lock_data.unlock_calls = 0;
    static int my_sentinel_udata_value = 42; // Just some unique address/value
    g_lock_data.expected_udata = &my_sentinel_udata_value;
    g_lock_data.actual_udata = NULL;

    // Set the lock function
    ulog_set_lock(my_mock_lock_fn, g_lock_data.expected_udata);

    // Call a logging function, which should trigger the lock/unlock
    printf("Calling log_info, expecting lock/unlock calls...\n");
    log_info("Test message for ulog_set_lock.");

    // Assertions
    assert(g_lock_data.lock_calls == 1 && "Lock was not called exactly once.");
    printf("Lock calls: %d (Expected 1)\n", g_lock_data.lock_calls);

    assert(g_lock_data.unlock_calls == 1 && "Unlock was not called exactly once.");
    printf("Unlock calls: %d (Expected 1)\n", g_lock_data.unlock_calls);

    assert(g_lock_data.actual_udata == g_lock_data.expected_udata && "User data passed to lock function was incorrect.");
    printf("User data: Actual %p, Expected %p\n", g_lock_data.actual_udata, g_lock_data.expected_udata);
    
    // Check the final lock state. After a log operation, it should be unlocked.
    assert(g_lock_data.locked == false && "Lock was not released after log_info call.");
    printf("Final lock state: %s (Expected false/unlocked)\n", g_lock_data.locked ? "true/locked" : "false/unlocked");

    // Test that setting lock to NULL disables it
    ulog_set_lock(NULL, NULL);
    g_lock_data.lock_calls = 0; // Reset for this part of the test
    g_lock_data.unlock_calls = 0;
    log_info("Another test message after lock disabled.");
    assert(g_lock_data.lock_calls == 0 && "Lock was called after being disabled.");
    assert(g_lock_data.unlock_calls == 0 && "Unlock was called after being disabled.");
    printf("Locking disabled: Lock calls: %d, Unlock calls: %d (Expected 0 for both)\n", g_lock_data.lock_calls, g_lock_data.unlock_calls);


    printf("test_ulog_set_lock_functionality: Passed.\n\n");
}

int main() {
    printf("Starting unit tests for microlog thread safety (ulog_set_lock)...\n\n");
    
    // Set a global log level if necessary (e.g., to ensure log_info processes)
    ulog_set_level(LOG_TRACE); 

    test_ulog_set_lock_functionality();

    printf("All thread safety tests completed successfully!\n");
    return 0;
}
