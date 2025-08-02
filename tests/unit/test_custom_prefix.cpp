#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"
#include <cstring>

static void test_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    (void)ev;
    snprintf(prefix, prefix_size, "[PREFIX]");
}

struct PrefixTestFixture {
    PrefixTestFixture() {
        ulog_set_level(ULOG_DEBUG);
        ulog_set_quiet(false);
        ulog_add_callback(ut_callback, nullptr, ULOG_DEBUG);
        ut_callback_reset();
        ulog_set_prefix_fn(test_prefix);
    }
    ~PrefixTestFixture() = default;
};

TEST_CASE_FIXTURE(PrefixTestFixture, "Custom Prefix") {
    log_info("Prefix test");
    CHECK(strstr(ut_callback_get_last_message(), "Prefix test") != nullptr);
    CHECK(strstr(ut_callback_get_last_message(), "[PREFIX]") != nullptr);
}
