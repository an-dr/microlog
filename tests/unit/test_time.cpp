#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"
#include <cstring>
#include <time.h>

static void test_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PREFIX]");
}

struct TimeTestFixture {
  public:
    static bool callback_is_set;

    TimeTestFixture() {
        // Per-test setup
        if (!callback_is_set) {
            ulog_add_callback(ut_callback, nullptr, LOG_TRACE);
        }
        ulog_set_quiet(false);
        ut_callback_reset();
    }
    ~TimeTestFixture() = default;
};

bool TimeTestFixture::callback_is_set = false;

static void _get_time_bounds(time_t &before, time_t &after) {
    before = time(NULL);
    REQUIRE(before != (time_t)(-1));
    log_info("Time test");
    after = time(NULL);
    REQUIRE(after != (time_t)(-1));
}

static void _check_time_fields(int hh, int mm, int ss, const time_t &before,
                               const time_t &after) {
    auto before_tm = gmtime(&before);
    CHECK_GE(hh, before_tm->tm_hour);
    CHECK_GE(mm, before_tm->tm_min);
    CHECK_GE(ss, before_tm->tm_sec);

    auto after_tm = gmtime(&after);
    CHECK_LE(hh, after_tm->tm_hour);
    CHECK_LE(mm, after_tm->tm_min);
    CHECK_LE(ss, after_tm->tm_sec);
}

static void _check_date_time_fields(int yr, int mo, int d, int hh, int mm,
                                    int ss, const time_t &before,
                                    const time_t &after) {
    auto before_tm = gmtime(&before);
    CHECK_GE(yr, before_tm->tm_year + 1900);
    CHECK_GE(mo, before_tm->tm_mon + 1);
    CHECK_GE(d, before_tm->tm_mday);
    _check_time_fields(hh, mm, ss, before, after);

    auto after_tm = gmtime(&after);
    CHECK_LE(yr, after_tm->tm_year + 1900);
    CHECK_LE(mo, after_tm->tm_mon + 1);
    CHECK_LE(d, after_tm->tm_mday);
}

void _check_console_time() {
    time_t before, after;
    _get_time_bounds(before, after);

    // Make sure date is not present
    REQUIRE(sscanf(ut_callback_get_last_message(), "%*d-%*d-%*d ") == 0);

    // Parse time values, ensure 3 matches
    int hh, mm, ss;
    REQUIRE(sscanf(ut_callback_get_last_message(), "%d:%d:%d ", &hh, &mm,
                   &ss) == 3);

#ifdef FEATURE_CUSTOM_PREFIX
    // Should be no space after time
    REQUIRE(ut_callback_get_last_message()[9] != ' ');
#else   // FEATURE_CUSTOM_PREFIX
    // If no custom prefix, there should be a space after the time
    REQUIRE(ut_callback_get_last_message()[9] == ' ');
#endif  // FEATURE_CUSTOM_PREFIX

    _check_time_fields(hh, mm, ss, before, after);
}

void _check_file_time() {
    const char *filename = "test_output.log";
    FILE *fp = fopen(filename, "w");
    REQUIRE(fp != nullptr);
    ulog_add_fp(fp, LOG_INFO);

    time_t before, after;
    _get_time_bounds(before, after);

    fclose(fp);

    // Check if the file was created
    fp = fopen(filename, "r");
    REQUIRE(fp != nullptr);

    // Read entry
    char buffer[256];
    fgets(buffer, sizeof(buffer), fp);
    fclose(fp);

    // Parse time values, ensure 6 matches
    int yr, mo, d, hh, mm, ss;
    REQUIRE(sscanf(buffer, "%d-%d-%d %d:%d:%d ", &yr, &mo, &d, &hh, &mm, &ss) ==
            6);

    _check_date_time_fields(yr, mo, d, hh, mm, ss, before, after);
}

TEST_CASE_FIXTURE(TimeTestFixture, "Check time without prefix") {
    _check_console_time();
    _check_file_time();
}
#ifdef FEATURE_CUSTOM_PREFIX
TEST_CASE_FIXTURE(TimeTestFixture, "Check time with prefix") {
    ulog_set_prefix_fn(test_prefix);

    _check_console_time();
    _check_file_time();
}
#endif  // FEATURE_CUSTOM_PREFIX
