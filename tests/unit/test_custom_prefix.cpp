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
        ulog_set_level(LOG_TRACE);
        ulog_set_quiet(false);
        ulog_add_callback(ut_callback, nullptr, LOG_TRACE);
        ut_callback_reset();
        ulog_set_prefix_fn(test_prefix);
    }
    ~PrefixTestFixture() = default;
};

TEST_CASE_FIXTURE(PrefixTestFixture, "Custom Prefix") {
    log_info("Prefix test");
    CHECK(strstr(ut_callback_get_last_message(), "Prefix test") != nullptr);
    // Note: ut_callback only captures the message, not the prefix, so this is a smoke test
}
