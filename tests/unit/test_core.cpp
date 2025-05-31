#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "ulog.h"
#include "ut_callback.h"

struct CoreTestFixture {
public:
    static bool callback_is_set;
    
    CoreTestFixture() {
        // Per-test setup
        if (!callback_is_set) {
            ulog_add_callback(ut_callback, nullptr, LOG_TRACE);
            callback_is_set = true;
        }
        ulog_set_level(LOG_TRACE);
        ulog_set_quiet(false);
        ut_callback_reset();
    }

    ~CoreTestFixture() = default;
};

bool CoreTestFixture::callback_is_set = false;


TEST_CASE_FIXTURE(CoreTestFixture, "Base") {
    log_trace("This is a TRACE message: %d", 123);
    log_debug("This is a DEBUG message: %s", "test");
    log_info("This is an INFO message: %.2f", 1.23);
    log_warn("This is a WARN message");
    log_error("This is an ERROR message: %x", 0xff);
    log_fatal("This is a FATAL message");

    CHECK(ut_callback_get_message_count() == 6);
    CHECK(strcmp(ut_callback_get_last_message(), "This is a FATAL message") == 0);
}

TEST_CASE_FIXTURE(CoreTestFixture, "Levels") {
    ulog_set_level(LOG_INFO);

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

TEST_CASE_FIXTURE(CoreTestFixture, "Quiet Mode") {
    ulog_set_quiet(true);
    log_info("This message will trigger extra callbacks, stdout should be quiet.");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_set_quiet(false);
    log_info("This message will trigger extra callbacks, stdout is not quiet.");
    CHECK(ut_callback_get_message_count() == 2);
}
