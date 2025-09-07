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
            ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
            callback_is_set = true;
        }
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        ut_callback_reset();
    }

    ~TestFixture() = default;
};

bool TestFixture::callback_is_set = false;

TEST_CASE_FIXTURE(TestFixture, "Topics: Enable/Disable and Levels") {
    ulog_topic_add("testtopic", ULOG_OUTPUT_ALL, true);

    ulog_t_trace("testtopic",
               "Topic enabled - At default topic level - should appear");
    CHECK(ut_callback_get_message_count() == 1);

    ulog_t_error("testtopic", "Above default topic level - should appear");
    CHECK(ut_callback_get_message_count() == 2);

    ulog_topic_disable("testtopic");
    ulog_t_info("testtopic", "Should not appear");
    CHECK(ut_callback_get_message_count() == 2);

    ulog_topic_enable("testtopic");
    ulog_topic_level_set("testtopic", ULOG_LEVEL_ERROR);
    ulog_t_warn("testtopic", "Below topic level - should not appear");
    CHECK(ut_callback_get_message_count() == 2);

    ulog_t_error("testtopic", "At topic level - should appear");
    CHECK(ut_callback_get_message_count() == 3);

    ulog_topic_level_set("testtopic", ULOG_LEVEL_TRACE);
    ulog_t_trace("testtopic", "At topic level - should appear");
    CHECK(ut_callback_get_message_count() == 4);

    ulog_topic_disable("testtopic");
    ulog_t_info("testtopic", "Should not appear again");
    CHECK(ut_callback_get_message_count() == 4);

    ulog_topic_enable("testtopic");
    ulog_t_info("testtopic", "Topic re-enabled and should appear");
    CHECK(ut_callback_get_message_count() == 5);

    ulog_topic_level_set("testtopic", ULOG_LEVEL_INFO);
    ulog_t_info("testtopic", "Topic level set to INFO and should appear");
    CHECK(ut_callback_get_message_count() == 6);

    ulog_topic_disable_all();
    ulog_t_info("testtopic", "Should not appear after disabling all topics");
    CHECK(ut_callback_get_message_count() == 6);

    ulog_topic_enable_all();
    ulog_t_info("testtopic", "Should appear after enabling all topics");
    CHECK(ut_callback_get_message_count() == 7);
}

TEST_CASE_FIXTURE(TestFixture, "Topics: Cannot create topic with empty name") {
    int res;

    res = ulog_topic_add(NULL, ULOG_OUTPUT_ALL, true);
    CHECK(res == -1);
}

TEST_CASE_FIXTURE(TestFixture, "Topics: Cannot create duplicate") {
    int res;

    // Check that the topic created in previous test fixture is still available
    res = ulog_topic_get_id("testtopic");
    CHECK(res == 0);
    ulog_t_info(
        "testtopic",
        "Should appear because the topic already exists and level set to INFO");
    CHECK(ut_callback_get_message_count() == 1);

    // Check that re-adding an existing topic does not duplicate it (thanks to
    // topic ID returned by ulog_topic_add)
    res = ulog_topic_add("testtopic", ULOG_OUTPUT_ALL, true);
    CHECK(res == 0);
    res = ulog_topic_add("testtopic_2", ULOG_OUTPUT_ALL, true);
    CHECK(res == 1);
    ulog_t_error("testtopic_2",
               "Should appear because there is still one free slot");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(TestFixture,
                  "Topics: Cannot create more than ULOG_BUILD_TOPICS_NUM topics") {
    int res;

    res = ulog_topic_add("testtopic_3", ULOG_OUTPUT_ALL, true);
    CHECK(res == -1);
    ulog_t_error("testtopic_3",
               "Should not appear because static allocation is set to 2 slots");
    CHECK(ut_callback_get_message_count() == 0);
}

TEST_CASE_FIXTURE(TestFixture, "Topics: Remove existing topic and re-add within static capacity") {
    // Precondition: earlier tests created 'testtopic' (id 0) and 'testtopic_2' (id 1)
    int existing_id = ulog_topic_get_id("testtopic_2");
    CHECK(existing_id == 1);

    // Log to ensure it works
    ulog_t_error("testtopic_2", "Before removal");
    CHECK(ut_callback_get_message_count() == 1);

    // Remove the topic
    ulog_status st = ulog_topic_remove("testtopic_2");
    CHECK(st == ULOG_STATUS_OK);

    // Logging now should be suppressed (topic gone)
    ulog_t_error("testtopic_2", "After removal (suppressed)");
    CHECK(ut_callback_get_message_count() == 1);

    // Second removal should return NOT_FOUND
    st = ulog_topic_remove("testtopic_2");
    CHECK(st == ULOG_STATUS_NOT_FOUND);

    // Re-add topic (should reuse freed slot, id expected 1 but just check valid)
    int new_id = ulog_topic_add("testtopic_2", ULOG_OUTPUT_ALL, true);
    CHECK(new_id != ULOG_TOPIC_ID_INVALID);

    // Logging again should appear
    ulog_t_error("testtopic_2", "After re-add");
    CHECK(ut_callback_get_message_count() == 2);
}
