#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <chrono>
#include <ctime>
#include "ulog.h"
#include "ut_callback.h"

struct TestFixture {
  public:
    static bool callback_is_set;

    TestFixture() {
        // Per-test setup
        if (!callback_is_set) {
            ulog_user_callback_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
            callback_is_set = true;
        }
        ut_callback_reset();
    }

    ~TestFixture() = default;
};

bool TestFixture::callback_is_set = false;

TEST_CASE_FIXTURE(TestFixture, "Default level") {
    log_trace("This TRACE should not be processed.");
    CHECK(ut_callback_get_message_count() == 0);
    log_debug("This DEBUG should be processed.");
    CHECK(ut_callback_get_message_count() == 1);
    log_info("This INFO should be processed.");
    CHECK(ut_callback_get_message_count() == 2);
    log_warn("This WARN should be processed.");
    CHECK(ut_callback_get_message_count() == 3);
    log_error("This ERROR should be processed.");
    CHECK(ut_callback_get_message_count() == 4);
    log_fatal("This FATAL should be processed.");
    CHECK(ut_callback_get_message_count() == 5);
}

TEST_CASE_FIXTURE(TestFixture, "Base") {
    ulog_output_set_level(ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);

    log_trace("This is a TRACE message: %d", 123);
    log_debug("This is a DEBUG message: %s", "test");
    log_info("This is an INFO message: %.2f", 1.23);
    log_warn("This is a WARN message");
    log_error("This is an ERROR message: %x", 0xff);
    log_fatal("This is a FATAL message");

    CHECK(ut_callback_get_message_count() == 6);
    CHECK(strstr(ut_callback_get_last_message(), "This is a FATAL message") !=
          nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Levels") {
    ulog_output_set_level(ULOG_OUTPUT_ALL, ULOG_LEVEL_INFO);

    log_trace("This TRACE should not be processed.");
    CHECK(ut_callback_get_message_count() == 0);
    log_debug("This DEBUG should not be processed.");
    CHECK(ut_callback_get_message_count() == 0);
    log_info("This INFO should be processed.");
    CHECK(ut_callback_get_message_count() == 1);
    log_warn("This WARN should be processed.");
    CHECK(ut_callback_get_message_count() == 2);
    log_error("This ERROR should be processed.");
    CHECK(ut_callback_get_message_count() == 3);
    log_fatal("This FATAL should be processed.");
    CHECK(ut_callback_get_message_count() == 4);
}

TEST_CASE_FIXTURE(TestFixture, "File Output") {
    const char *filename = "test_output.log";
    FILE *fp             = fopen(filename, "w");
    REQUIRE(fp != nullptr);
    ulog_user_callback_add_fp(fp, ULOG_LEVEL_INFO);

    log_info("This is an INFO message to file.");
    fclose(fp);

    // Check if the file was created and contains the expected message
    fp = fopen(filename, "r");
    REQUIRE(fp != nullptr);

    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    CHECK(strstr(buffer, "This is an INFO message to file.") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Performance") {
    //Current time
    auto start = std::chrono::high_resolution_clock::now();
    int iterations = 100000;
    for (int i = 0; i < iterations; ++i) {
        log_fatal("This is a FATAL message with topic");
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    float average_log_time = duration_us / iterations;
    // Check if the duration is within acceptable limits
    CHECK(duration_us < 1000000); // 1 second for 100000 iterations
    CHECK(ut_callback_get_message_count() == iterations);
    // Check if the average log time is reasonable
    CHECK(average_log_time < 1000); // Less than 1 millisecond per log

    printf("Logging 100000 messages took: %ld microseconds (%f per log)\n", duration_us, average_log_time);
}
