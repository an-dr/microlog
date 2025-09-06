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

    // Additional: empty format string
    ulog_info("");

    CHECK(ut_callback_get_message_count() == 7);
    CHECK(strstr(ut_callback_get_last_message(), "INFO") != nullptr);
    CHECK(std::string(ulog_level_to_string(ULOG_LEVEL_DEBUG)) == "DEBUG");
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
    ulog_output_id fid = ulog_output_add_file(fp, ULOG_LEVEL_INFO);

    ulog_info("This is an INFO message to file.");
    fclose(fp);

    // Attempt to remove stdout should fail; removing file output should succeed
    CHECK(ulog_output_remove(ULOG_OUTPUT_STDOUT) == ULOG_STATUS_ERROR);
    CHECK(ulog_output_remove(fid) == ULOG_STATUS_OK);

    // Check file content
    fp = fopen(filename, "r");
    REQUIRE(fp != nullptr);

    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    CHECK(strstr(buffer, "This is an INFO message to file.") != nullptr);
}
