#include "ut_test_suite.h"
#include <stdio.h>
#include <string.h>

#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))

void TestSuite_run(TestSuite *suite) {
    if (suite->setup_suite) {
        suite->setup_suite();
    }

    for (int i = 0; i < suite->test_count; i++) {
        if (suite->setup_test) {
            suite->setup_test();
        }
        printf("[START] Test: %s\n", suite->test_names[i]);
        if (suite->tests[i]) {
            suite->tests[i]();
        }
        printf("[DONE ] Test: %s\n", suite->test_names[i]);
        if (suite->teardown_test) {
            suite->teardown_test();
        }
    }

    if (suite->teardown_suite) {
        suite->teardown_suite();
    }
}

void TestSuite_add_test(TestSuite *suite, const char *test_name,
                        TestFunction test) {
    int i = suite->test_count;
    if (i < COUNT_OF(suite->tests)) {
        suite->tests[i] = test;

        strncpy(suite->test_names[i], test_name, TEST_NAME_MAX_LENGTH - 1);

        // Ensure null-termination
        suite->test_names[i][TEST_NAME_MAX_LENGTH - 1] = '\0';

        // Increment the test count
        suite->test_count++;
    }
}

void TestSuite_init(TestSuite *suite, void (*setup_suite)(),
                    void (*teardown_suite)(), void (*setup_test)(),
                    void (*teardown_test)()) {
    suite->setup_suite    = setup_suite;
    suite->teardown_suite = teardown_suite;
    suite->setup_test     = setup_test;
    suite->teardown_test  = teardown_test;
    suite->test_count     = 0;
}
