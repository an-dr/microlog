#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"

struct TopicsTestFixture {
    TopicsTestFixture() {
        ulog_set_level(LOG_TRACE);
        ulog_set_quiet(false);
        ulog_add_callback(ut_callback, nullptr, LOG_TRACE);
        ut_callback_reset();
    }
    ~TopicsTestFixture() = default;
};

TEST_CASE_FIXTURE(TopicsTestFixture, "Topics: Enable/Disable and Levels") {
#if FEATURE_TOPICS
    ulog_add_topic("testtopic", true);
    logt_info("testtopic", "Topic enabled");
    CHECK(ut_callback_get_message_count() == 1);
    ulog_disable_topic("testtopic");
    logt_info("testtopic", "Should not appear");
    CHECK(ut_callback_get_message_count() == 1);
    ulog_enable_topic("testtopic");
    ulog_set_topic_level("testtopic", LOG_ERROR);
    logt_warn("testtopic", "Below topic level");
    CHECK(ut_callback_get_message_count() == 1);
    logt_error("testtopic", "At topic level");
    CHECK(ut_callback_get_message_count() == 2);
#endif
}
