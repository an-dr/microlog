#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "ulog.h"
#include "ut_callback.h"

struct TestFixture {
public:
    static bool callback_is_set;
    
    TestFixture() {
        // Per-test setup
        if (!callback_is_set) {
            ulog_add_callback(ut_callback, nullptr, LOG_TRACE);
            callback_is_set = true;
        }
        ulog_set_level(LOG_TRACE);
        ulog_set_quiet(false);
        ut_callback_reset();
    }

    ~TestFixture() = default;
};

bool TestFixture::callback_is_set = false;

TEST_CASE_FIXTURE(TestFixture, "Topics: Enable/Disable and Levels") {
    ulog_add_topic("testtopic", true);
    logt_warn("testtopic", "Topic enabled - At default topic level - should appear");
    CHECK(ut_callback_get_message_count() == 1);
    
    logt_info("testtopic", "Below default topic level - should not appear");
    CHECK(ut_callback_get_message_count() == 1);

    logt_error("testtopic", "Above default topic level - should appear");
    CHECK(ut_callback_get_message_count() == 2);

    ulog_disable_topic("testtopic");
    logt_info("testtopic", "Should not appear");
    CHECK(ut_callback_get_message_count() == 2);
    
    ulog_enable_topic("testtopic");
    ulog_set_topic_level("testtopic", LOG_ERROR);
    logt_warn("testtopic", "Below topic level - should not appear");
    CHECK(ut_callback_get_message_count() == 2);
    
    logt_error("testtopic", "At topic level - should appear");
    CHECK(ut_callback_get_message_count() == 3);
    
    ulog_set_topic_level("testtopic", LOG_TRACE);
    logt_trace("testtopic", "At topic level - should appear");
    CHECK(ut_callback_get_message_count() == 4);
    
    ulog_disable_topic("testtopic");
    logt_info("testtopic", "Should not appear again");
    CHECK(ut_callback_get_message_count() == 4);
    
    ulog_enable_topic("testtopic");
    logt_info("testtopic", "Topic re-enabled and should appear");
    CHECK(ut_callback_get_message_count() == 5);
    
    ulog_set_topic_level("testtopic", LOG_INFO);
    logt_info("testtopic", "Topic level set to INFO and should appear");
    CHECK(ut_callback_get_message_count() == 6);
    
    ulog_disable_all_topics();
    logt_info("testtopic", "Should not appear after disabling all topics");
    CHECK(ut_callback_get_message_count() == 6);
    
    ulog_enable_all_topics();
    logt_info("testtopic", "Should appear after enabling all topics");
    CHECK(ut_callback_get_message_count() == 7);
}
