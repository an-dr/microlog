#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include <cstring>
#include "ulog.h"
#include "ut_callback.h"

// Cover rarely used public event accessors and error branches.
struct EventApiFixture {
    EventApiFixture() {
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
        ut_callback_reset();
    }
};

TEST_CASE_FIXTURE(EventApiFixture, "Event API: getters and message extract") {
    ulog_info("Event API test %d", 42);
    REQUIRE(ut_callback_get_message_count() == 1);

    // Build a synthetic event by intercepting via callback is not exposed; so
    // we trigger another log and directly verify formatting helpers using the
    // last captured string (indirect coverage already for ulog_event_to_cstr).

    // Directly test ulog_event_get_message error handling with nullptrs.
    // We can't construct ulog_event outside library, but we can still call the
    // function with NULL to hit invalid argument path.
    char buffer[8];
    CHECK(ulog_event_get_message(nullptr, buffer, sizeof(buffer)) == ULOG_STATUS_INVALID_ARGUMENT);
    CHECK(ulog_event_get_message((ulog_event*)nullptr, nullptr, 0) == ULOG_STATUS_INVALID_ARGUMENT);
}

TEST_CASE_FIXTURE(EventApiFixture, "Output remove: invalid handle paths") {
    // Removing stdout should fail
    CHECK(ulog_output_remove(ULOG_OUTPUT_STDOUT) == ULOG_STATUS_ERROR);
    // Removing invalid id should return invalid arg
    CHECK(ulog_output_remove(-123) == ULOG_STATUS_INVALID_ARGUMENT);
}
