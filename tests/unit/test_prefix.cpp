#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include <cstring>
#include "ulog.h"
#include "ut_callback.h"

static void test_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PREFIX]");
}

struct PrefixTestFixture {
    PrefixTestFixture() {
        ulog_output_level_set(ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ut_callback_reset();
        ulog_prefix_set_fn(test_prefix);
    }
    ~PrefixTestFixture() = default;
};

TEST_CASE_FIXTURE(PrefixTestFixture, "Prefix") {
    log_info("Prefix test");
    CHECK(strstr(ut_callback_get_last_message(), "Prefix test") != nullptr);
    CHECK(strstr(ut_callback_get_last_message(), "[PREFIX]") != nullptr);
}
