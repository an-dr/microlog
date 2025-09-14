//  unit tests for dynamic topics feature
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

extern "C" {
#include "ulog.h"
#include "ut_callback.h"
}

#include <cstdio>
#include <cstring>

// Test fixture for dynamic topics tests
struct DynamicTopicsTestFixture {
    DynamicTopicsTestFixture() {
        ut_callback_reset();
        ulog_cleanup();
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
        
        // Set up callback to capture log messages
        ulog_output_add(ut_callback, nullptr, ULOG_LEVEL_TRACE);
    }
    
    ~DynamicTopicsTestFixture() {
        ulog_cleanup();
    }
};

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Add") {
    // Test adding topics with dynamic allocation
    ulog_topic_id topic1 = ulog_topic_add("network", ULOG_OUTPUT_ALL, true);
    CHECK(topic1 != ULOG_TOPIC_ID_INVALID);
    
    ulog_topic_id topic2 = ulog_topic_add("storage", ULOG_OUTPUT_ALL, false);
    CHECK(topic2 != ULOG_TOPIC_ID_INVALID);
    
    // Topics should have different IDs
    CHECK(topic1 != topic2);
    
    // Test that duplicate topic names return the same ID
    ulog_topic_id topic1_dup = ulog_topic_add("network", ULOG_OUTPUT_ALL, true);
    CHECK(topic1_dup == topic1);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Get ID") {
    // Test getting non-existent topic ID
    ulog_topic_id missing_id = ulog_topic_get_id("missing");
    CHECK(missing_id == ULOG_TOPIC_ID_INVALID);
    
    // Test with empty topic name
    ulog_topic_id empty_id = ulog_topic_get_id("");
    CHECK(empty_id == ULOG_TOPIC_ID_INVALID);
    
    // Add a topic and then try to get its ID
    ulog_topic_id added_id = ulog_topic_add("testnet", ULOG_OUTPUT_ALL, true);
    REQUIRE(added_id != ULOG_TOPIC_ID_INVALID);
    
    // Try to get the ID of the topic we just added
    ulog_topic_id found_id = ulog_topic_get_id("testnet");
    CHECK(found_id == added_id);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Logging") {
    // Add topics with different states
    ulog_topic_id enabled_topic = ulog_topic_add("enabled", ULOG_OUTPUT_ALL, true);
    ulog_topic_id disabled_topic = ulog_topic_add("disabled", ULOG_OUTPUT_ALL, false);
    REQUIRE(enabled_topic != ULOG_TOPIC_ID_INVALID);
    REQUIRE(disabled_topic != ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // Log to enabled topic - should appear
    ulog_topic_info("enabled", "This should appear");
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "[enabled]") != nullptr);
    CHECK(strstr(ut_callback_get_last_message(), "This should appear") != nullptr);
    
    // Log to disabled topic - should not appear
    ulog_topic_info("disabled", "This should not appear");
    CHECK(ut_callback_get_message_count() == 1); // Count should remain the same
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Enable/Disable") {
    ulog_topic_id topic = ulog_topic_add("toggle", ULOG_OUTPUT_ALL, false);
    REQUIRE(topic != ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // Initially disabled - should not log
    ulog_topic_info("toggle", "Message 1");
    CHECK(ut_callback_get_message_count() == 0);
    
    // Enable topic
    ulog_status result = ulog_topic_enable("toggle");
    CHECK(result == ULOG_STATUS_OK);
    
    // Now should log
    ulog_topic_info("toggle", "Message 2");
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "[toggle]") != nullptr);
    
    // Disable topic again
    result = ulog_topic_disable("toggle");
    CHECK(result == ULOG_STATUS_OK);
    
    // Should not log anymore
    ulog_topic_info("toggle", "Message 3");
    CHECK(ut_callback_get_message_count() == 1); // Count should remain the same
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Level Setting") {
    ulog_topic_id topic = ulog_topic_add("leveltest", ULOG_OUTPUT_ALL, true);
    REQUIRE(topic != ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // By default, topic level is TRACE, so all levels should work
    ulog_topic_trace("leveltest", "Trace message");
    CHECK(ut_callback_get_message_count() == 1);
    
    ulog_topic_debug("leveltest", "Debug message");
    CHECK(ut_callback_get_message_count() == 2);
    
    ulog_topic_info("leveltest", "Info message");
    CHECK(ut_callback_get_message_count() == 3);
    
    // Set topic level to WARN
    ulog_status result = ulog_topic_level_set("leveltest", ULOG_LEVEL_WARN);
    CHECK(result == ULOG_STATUS_OK);
    
    ut_callback_reset();
    
    // Now TRACE, DEBUG, INFO should be filtered out
    ulog_topic_trace("leveltest", "Filtered trace");
    ulog_topic_debug("leveltest", "Filtered debug");
    ulog_topic_info("leveltest", "Filtered info");
    CHECK(ut_callback_get_message_count() == 0);
    
    // But WARN and above should work
    ulog_topic_warn("leveltest", "Warning message");
    CHECK(ut_callback_get_message_count() == 1);
    
    ulog_topic_error("leveltest", "Error message");
    CHECK(ut_callback_get_message_count() == 2);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Enable/Disable All") {
    // Add multiple topics
    ulog_topic_id topic1 = ulog_topic_add("topic1", ULOG_OUTPUT_ALL, true);
    ulog_topic_id topic2 = ulog_topic_add("topic2", ULOG_OUTPUT_ALL, false);
    ulog_topic_id topic3 = ulog_topic_add("topic3", ULOG_OUTPUT_ALL, true);
    
    REQUIRE(topic1 != ULOG_TOPIC_ID_INVALID);
    REQUIRE(topic2 != ULOG_TOPIC_ID_INVALID);
    REQUIRE(topic3 != ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // topic1 and topic3 should be enabled, topic2 disabled
    ulog_topic_info("topic1", "Message 1");
    ulog_topic_info("topic2", "Message 2"); // should be filtered
    ulog_topic_info("topic3", "Message 3");
    CHECK(ut_callback_get_message_count() == 2);
    
    // Disable all topics
    ulog_status result = ulog_topic_disable_all();
    CHECK(result == ULOG_STATUS_OK);
    
    ut_callback_reset();
    
    // All should be disabled now
    ulog_topic_info("topic1", "Message 4");
    ulog_topic_info("topic2", "Message 5");
    ulog_topic_info("topic3", "Message 6");
    CHECK(ut_callback_get_message_count() == 0);
    
    // Enable all topics
    result = ulog_topic_enable_all();
    CHECK(result == ULOG_STATUS_OK);
    
    ut_callback_reset();
    
    // All should be enabled now
    ulog_topic_info("topic1", "Message 7");
    ulog_topic_info("topic2", "Message 8");
    ulog_topic_info("topic3", "Message 9");
    CHECK(ut_callback_get_message_count() == 3);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Remove") {
    // Add a topic
    ulog_topic_id topic = ulog_topic_add("removeme", ULOG_OUTPUT_ALL, true);
    REQUIRE(topic != ULOG_TOPIC_ID_INVALID);
    
    // Verify it exists
    ulog_topic_id found = ulog_topic_get_id("removeme");
    CHECK(found == topic);
    
    ut_callback_reset();
    
    // Verify it works
    ulog_topic_info("removeme", "Before removal");
    CHECK(ut_callback_get_message_count() == 1);
    
    // Remove the topic
    ulog_status result = ulog_topic_remove("removeme");
    CHECK(result == ULOG_STATUS_OK);
    
    // Verify it's gone
    ulog_topic_id missing = ulog_topic_get_id("removeme");
    CHECK(missing == ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // Should not log anymore
    ulog_topic_info("removeme", "After removal");
    CHECK(ut_callback_get_message_count() == 0);
    
    // Test removing non-existent topic
    result = ulog_topic_remove("nonexistent");
    CHECK(result == ULOG_STATUS_NOT_FOUND);
    
    // Test removing with invalid arguments
    result = ulog_topic_remove("");
    CHECK(result == ULOG_STATUS_INVALID_ARGUMENT);
    
    result = ulog_topic_remove(nullptr);
    CHECK(result == ULOG_STATUS_INVALID_ARGUMENT);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Output Assignment") {
    // Test that topics can be assigned to specific outputs
    FILE *temp_file = tmpfile();
    REQUIRE(temp_file != nullptr);
    
    ulog_output_id file_output = ulog_output_add_file(temp_file, ULOG_LEVEL_TRACE);
    REQUIRE(file_output != ULOG_OUTPUT_INVALID);
    
    // Add topic assigned to file output only
    ulog_topic_id topic = ulog_topic_add("fileonly", file_output, true);
    REQUIRE(topic != ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // Log to this topic - should not appear in our callback (which is stdout)
    // but should appear in the file
    ulog_topic_info("fileonly", "File only message");
    CHECK(ut_callback_get_message_count() == 0);
    
    // Test adding topic to all outputs
    ulog_topic_id all_topic = ulog_topic_add("alloutputs", ULOG_OUTPUT_ALL, true);
    REQUIRE(all_topic != ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // This should appear in our callback
    ulog_topic_info("alloutputs", "All outputs message");
    CHECK(ut_callback_get_message_count() == 1);
    
    fclose(temp_file);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic New Topic Behavior") {
    // Test default behavior for new topics
    ut_callback_reset();
    
    // Initially, new topics should be disabled by default (and not exist)
    ulog_topic_info("newtopic1", "Should not appear");
    CHECK(ut_callback_get_message_count() == 0);
    
    // Enable all topics - this enables existing topics but doesn't auto-create new ones
    ulog_topic_enable_all();
    
    ut_callback_reset();
    
    // New topics still should not appear since they don't exist
    ulog_topic_info("newtopic2", "Should not appear");
    CHECK(ut_callback_get_message_count() == 0);
    
    // However, if we explicitly create a topic after enable_all, it should be enabled
    ulog_topic_id topic_id = ulog_topic_add("newtopic2", ULOG_OUTPUT_ALL, true);
    REQUIRE(topic_id != ULOG_TOPIC_ID_INVALID);
    
    ut_callback_reset();
    
    // Now it should appear
    ulog_topic_info("newtopic2", "Should appear");
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "[newtopic2]") != nullptr);
    
    // Disable all topics - this sets new topic default to disabled
    ulog_topic_disable_all();
    
    ut_callback_reset();
    
    // Existing topics should now be disabled
    ulog_topic_info("newtopic2", "Should not appear");
    CHECK(ut_callback_get_message_count() == 0);
    
    // And new topics still should not appear since they don't exist
    ulog_topic_info("newtopic3", "Should not appear");
    CHECK(ut_callback_get_message_count() == 0);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Invalid Operations") {
    // Test operations on non-existent topics
    
    ulog_status result = ulog_topic_enable("nonexistent");
    CHECK(result == ULOG_STATUS_NOT_FOUND);
    
    result = ulog_topic_disable("nonexistent");
    CHECK(result == ULOG_STATUS_NOT_FOUND);
    
    result = ulog_topic_level_set("nonexistent", ULOG_LEVEL_WARN);
    CHECK(result == ULOG_STATUS_NOT_FOUND);
    
    // Test with invalid topic names
    ulog_topic_id invalid_topic = ulog_topic_add("", ULOG_OUTPUT_ALL, true);
    CHECK(invalid_topic == ULOG_TOPIC_ID_INVALID);
    
    invalid_topic = ulog_topic_add(nullptr, ULOG_OUTPUT_ALL, true);
    CHECK(invalid_topic == ULOG_TOPIC_ID_INVALID);
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Memory Management") {
    // Test that we can add multiple topics without issues
    const int NUM_TOPICS = 10;  // Back to a reasonable number
    ulog_topic_id topics[NUM_TOPICS];
    
    for (int i = 0; i < NUM_TOPICS; i++) {
        char topic_name[32];
        snprintf(topic_name, sizeof(topic_name), "topic_%d", i);
        topics[i] = ulog_topic_add(topic_name, ULOG_OUTPUT_ALL, true);
        CHECK(topics[i] != ULOG_TOPIC_ID_INVALID);
    }
    
    // Verify all topics are accessible
    for (int i = 0; i < NUM_TOPICS; i++) {
        char topic_name[32];
        snprintf(topic_name, sizeof(topic_name), "topic_%d", i);
        ulog_topic_id found = ulog_topic_get_id(topic_name);
        CHECK(found == topics[i]);
    }
    
    ut_callback_reset();
    
    // Test logging to some of the topics
    ulog_topic_info("topic_0", "First topic");
    ulog_topic_info("topic_5", "Middle topic");
    ulog_topic_info("topic_9", "Last topic");
    CHECK(ut_callback_get_message_count() == 3);
    
    // Cleanup will remove all topics
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Configuration") {
    // Test topic configuration (requires ULOG_BUILD_DYNAMIC_CONFIG=1)
    // This test might not work in all configurations, but we can test the function calls
    
    // Topic config should be available in dynamic topics mode
    ulog_status result = ulog_topic_config(false);
    // The result depends on whether ULOG_BUILD_DYNAMIC_CONFIG is enabled
    // In our dynamic topics config, it should work
    CHECK((result == ULOG_STATUS_OK || result == ULOG_STATUS_DISABLED));
    
    if (result == ULOG_STATUS_OK) {
        // If topic config is available, test disabling/enabling topic display
        ulog_topic_id topic = ulog_topic_add("configtest", ULOG_OUTPUT_ALL, true);
        REQUIRE(topic != ULOG_TOPIC_ID_INVALID);
        
        ut_callback_reset();
        
        // With topic config disabled, topic names shouldn't appear in output
        ulog_topic_info("configtest", "Hidden topic");
        CHECK(ut_callback_get_message_count() == 1);
        // Message should appear but without [configtest] prefix
        CHECK(strstr(ut_callback_get_last_message(), "[configtest]") == nullptr);
        CHECK(strstr(ut_callback_get_last_message(), "Hidden topic") != nullptr);
        
        // Re-enable topic config
        result = ulog_topic_config(true);
        CHECK(result == ULOG_STATUS_OK);
        
        ut_callback_reset();
        
        // Now topic names should appear
        ulog_topic_info("configtest", "Visible topic");
        CHECK(ut_callback_get_message_count() == 1);
        CHECK(strstr(ut_callback_get_last_message(), "[configtest]") != nullptr);
        CHECK(strstr(ut_callback_get_last_message(), "Visible topic") != nullptr);
    }
}

TEST_CASE_FIXTURE(DynamicTopicsTestFixture, "Dynamic Topic Complex Scenario") {
    // Complex scenario combining multiple features
    
    // Set up multiple topics with different configurations
    ulog_topic_id net_topic = ulog_topic_add("network", ULOG_OUTPUT_ALL, true);
    ulog_topic_id db_topic = ulog_topic_add("database", ULOG_OUTPUT_ALL, false);
    ulog_topic_id ui_topic = ulog_topic_add("ui", ULOG_OUTPUT_ALL, true);
    
    REQUIRE(net_topic != ULOG_TOPIC_ID_INVALID);
    REQUIRE(db_topic != ULOG_TOPIC_ID_INVALID);
    REQUIRE(ui_topic != ULOG_TOPIC_ID_INVALID);
    
    // Set different levels for each topic
    ulog_topic_level_set("network", ULOG_LEVEL_DEBUG);
    ulog_topic_level_set("database", ULOG_LEVEL_WARN);
    ulog_topic_level_set("ui", ULOG_LEVEL_INFO);
    
    ut_callback_reset();
    
    // Test various combinations
    ulog_topic_trace("network", "Network trace"); // Should be filtered (below DEBUG)
    ulog_topic_debug("network", "Network debug"); // Should appear
    ulog_topic_info("database", "DB info");       // Should be filtered (topic disabled)
    ulog_topic_info("ui", "UI info");             // Should appear
    
    CHECK(ut_callback_get_message_count() == 2);
    
    // Enable database topic
    ulog_topic_enable("database");
    
    ut_callback_reset();
    
    // Test again
    ulog_topic_info("database", "DB info");       // Should be filtered (below WARN)
    ulog_topic_warn("database", "DB warning");    // Should appear
    ulog_topic_error("ui", "UI error");           // Should appear
    
    CHECK(ut_callback_get_message_count() == 2);
    
    // Test topic removal
    ulog_topic_remove("ui");
    
    ut_callback_reset();
    
    ulog_topic_info("ui", "UI after removal");    // Should not appear
    ulog_topic_warn("database", "DB still works"); // Should appear
    
    CHECK(ut_callback_get_message_count() == 1);
    CHECK(strstr(ut_callback_get_last_message(), "[database]") != nullptr);
}
