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
    logt_warn("testtopic",
              "Topic enabled - At default topic level - should appear");
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

TEST_CASE_FIXTURE(TestFixture, "Topics: Cannot create topic with empty name") {
    int res;

    res = ulog_add_topic(NULL, true);
    CHECK(res == -1);
}

TEST_CASE_FIXTURE(TestFixture, "Topics: Cannot create duplicate") {
    int res;

    // Check that the topic created in previous test fixture is still available
    res = ulog_get_topic_id("testtopic");
    CHECK(res == 0);
    logt_info(
        "testtopic",
        "Should appear because the topic already exists and level set to INFO");
    CHECK(ut_callback_get_message_count() == 1);

    // Check that re-adding an existing topic does not duplicate it (thanks to
    // topic ID returned by ulog_add_topic)
    res = ulog_add_topic("testtopic", true);
    CHECK(res == 0);
    res = ulog_add_topic("testtopic_2", true);
    CHECK(res == 1);
    logt_error("testtopic_2",
               "Should appear because there is still one free slot");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(TestFixture,
                  "Topics: Cannot create more than ULOG_TOPICS_NUM topics") {
    int res;

    res = ulog_add_topic("testtopic_3", true);
    CHECK(res == -1);
    logt_error("testtopic_3",
               "Should not appear because static allocation is set to 2 slots");
    CHECK(ut_callback_get_message_count() == 0);
}
