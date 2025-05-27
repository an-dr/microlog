#pragma once

#define TEST_NUM 32
#define TEST_NAME_MAX_LENGTH 64

typedef void (*TestFunction)();

typedef struct {
    void (*setup_suite)();
    void (*teardown_suite)();
    void (*setup_test)();
    void (*teardown_test)();
    int test_count;
    TestFunction tests[TEST_NUM];
    char test_names[TEST_NUM][TEST_NAME_MAX_LENGTH];
} TestSuite;

void TestSuite_init(TestSuite *suite, void (*setup_suite)(),
                    void (*teardown_suite)(), void (*setup_test)(),
                    void (*teardown_test)());

void TestSuite_add_test(TestSuite *suite, const char *test_name,
                        TestFunction test);

void TestSuite_run(TestSuite *suite);
