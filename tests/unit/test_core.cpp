#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <chrono>
#include <cstring>
#include <ctime>
#include "ulog.h"
#include "ut_callback.h"

struct TestFixture {
  public:
    static bool callback_is_set;

    TestFixture() {
        // Per-test setup
        if (!callback_is_set) {
            ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
            callback_is_set = true;
        }
        ut_callback_reset();
    }

    ~TestFixture() = default;
};

bool TestFixture::callback_is_set = false;

TEST_CASE_FIXTURE(TestFixture, "Base") {
    ulog_output_level_set_all(ULOG_LEVEL_TRACE);

    ulog_trace("This is a TRACE message: %d", 123);
    ulog_debug("This is a DEBUG message: %s", "test");
    ulog_info("This is an INFO message: %.2f", 1.23);
    ulog_warn("This is a WARN message");
    ulog_error("This is an ERROR message: %x", 0xff);
    ulog_fatal("This is a FATAL message");

    CHECK(ut_callback_get_message_count() == 6);
    CHECK(strstr(ut_callback_get_last_message(), "This is a FATAL message") !=
          nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Levels") {
    ulog_output_level_set_all(ULOG_LEVEL_INFO);

    ulog_trace("This TRACE should not be processed.");
    CHECK(ut_callback_get_message_count() == 0);
    ulog_debug("This DEBUG should not be processed.");
    CHECK(ut_callback_get_message_count() == 0);
    ulog_info("This INFO should be processed.");
    CHECK(ut_callback_get_message_count() == 1);
    ulog_warn("This WARN should be processed.");
    CHECK(ut_callback_get_message_count() == 2);
    ulog_error("This ERROR should be processed.");
    CHECK(ut_callback_get_message_count() == 3);
    ulog_fatal("This FATAL should be processed.");
    CHECK(ut_callback_get_message_count() == 4);
}

TEST_CASE_FIXTURE(TestFixture, "File Output") {
    const char *filename = "test_output.log";
    FILE *fp             = fopen(filename, "w");
    REQUIRE(fp != nullptr);
    ulog_output_add_file(fp, ULOG_LEVEL_INFO);

    ulog_info("This is an INFO message to file.");
    fclose(fp);

    // Check if the file was created and contains the expected message
    fp = fopen(filename, "r");
    REQUIRE(fp != nullptr);

    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    CHECK(strstr(buffer, "This is an INFO message to file.") != nullptr);
}

TEST_CASE_FIXTURE(TestFixture, "Invalid Level Handling") {
    // Test ulog_level_to_string with invalid levels
    const char *invalid_level_str = ulog_level_to_string((ulog_level)-1);
    CHECK(strcmp(invalid_level_str, "?") == 0);

    invalid_level_str = ulog_level_to_string((ulog_level)99);
    CHECK(strcmp(invalid_level_str, "?") == 0);

    // Test ulog_level_set_new_levels with NULL
    ulog_status result = ulog_level_set_new_levels(nullptr);
    CHECK(result == ULOG_STATUS_INVALID_ARGUMENT);

    // Test with invalid level descriptor (NULL names)
    ulog_level_descriptor invalid_desc = {
        .max_level = (ulog_level)6,
        .names = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr}
    };
    result = ulog_level_set_new_levels(&invalid_desc);
    CHECK(result == ULOG_STATUS_INVALID_ARGUMENT);

    // Test with invalid max_level
    const char *valid_names[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", nullptr, nullptr};
    ulog_level_descriptor invalid_max_desc = {
        .max_level = (ulog_level)0,  // Invalid max_level
        .names = {valid_names[0], valid_names[1], valid_names[2], valid_names[3], valid_names[4], valid_names[5], nullptr, nullptr}
    };
    result = ulog_level_set_new_levels(&invalid_max_desc);
    CHECK(result == ULOG_STATUS_INVALID_ARGUMENT);
}
