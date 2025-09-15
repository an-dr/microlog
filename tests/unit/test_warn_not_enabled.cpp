//  unit tests for warn not enabled feature
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

extern "C" {
#include "ulog.h"
#include "ut_callback.h"
}

#include <cstdio>
#include <cstring>

// Custom callback to capture warning messages
static int warning_callback_count = 0;
static char warning_last_message[512] = {0};

void warning_test_callback(ulog_event *ev, void *arg) {
    (void)arg;
    warning_callback_count++;
    
    if (ev && ulog_event_get_level(ev) == ULOG_LEVEL_WARN) {
        ulog_event_to_cstr(ev, warning_last_message, sizeof(warning_last_message));
    }
}

void reset_warning_callback() {
    warning_callback_count = 0;
    warning_last_message[0] = '\0';
}

// Test fixture for warn not enabled tests
struct WarnNotEnabledTestFixture {
    WarnNotEnabledTestFixture() {
        ut_callback_reset();
        reset_warning_callback();
        ulog_cleanup();
        ulog_output_level_set_all(ULOG_LEVEL_TRACE);
    }
    
    ~WarnNotEnabledTestFixture() {
        ulog_cleanup();
    }
};

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Color Config Warning") {
    // Add callback to capture warnings
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call color config function (should be disabled in base config and generate warning)
    ulog_status result = ulog_color_config(true);
    
    // Function should return DISABLED status
    CHECK(result == ULOG_STATUS_DISABLED);
    
    // Should generate a warning message
    CHECK(warning_callback_count == 1);
    CHECK(strstr(warning_last_message, "ulog_color_config") != nullptr);
    CHECK(strstr(warning_last_message, "ULOG_BUILD_COLOR disabled") != nullptr);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Prefix Config Warning") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call prefix config function (should be disabled and generate warning)
    ulog_status result = ulog_prefix_config(true);
    
    CHECK(result == ULOG_STATUS_DISABLED);
    CHECK(warning_callback_count == 1);
    CHECK(strstr(warning_last_message, "ulog_prefix_config") != nullptr);
    CHECK(strstr(warning_last_message, "ULOG_BUILD_DYNAMIC_CONFIG disabled") != nullptr);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Time Config Warning") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call time config function (should be disabled and generate warning)
    ulog_status result = ulog_time_config(true);
    
    CHECK(result == ULOG_STATUS_DISABLED);
    CHECK(warning_callback_count == 1);
    CHECK(strstr(warning_last_message, "ulog_time_config") != nullptr);
    CHECK(strstr(warning_last_message, "ULOG_BUILD_TIME disabled") != nullptr);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Source Location Config Warning") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call source location config function (should be disabled and generate warning)
    ulog_status result = ulog_source_location_config(false);
    
    CHECK(result == ULOG_STATUS_DISABLED);
    CHECK(warning_callback_count == 1);
    CHECK(strstr(warning_last_message, "ulog_source_location_config") != nullptr);
    CHECK(strstr(warning_last_message, "ULOG_BUILD_SOURCE_LOCATION disabled") != nullptr);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Level Config Warning") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call level config function (should be disabled and generate warning)
    ulog_status result = ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_SHORT);
    
    CHECK(result == ULOG_STATUS_DISABLED);
    CHECK(warning_callback_count == 1);
    CHECK(strstr(warning_last_message, "ulog_level_config") != nullptr);
    CHECK(strstr(warning_last_message, "ULOG_BUILD_DYNAMIC_CONFIG disabled") != nullptr);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Topic Config Warning") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call topic config function (should be disabled and generate warning)
    ulog_status result = ulog_topic_config(true);
    
    CHECK(result == ULOG_STATUS_DISABLED);
    CHECK(warning_callback_count == 1);
    CHECK(strstr(warning_last_message, "ulog_topic_config") != nullptr);
    CHECK(strstr(warning_last_message, "ULOG_BUILD_DYNAMIC_CONFIG disabled") != nullptr);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Topic Functions Warnings") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    SUBCASE("Topic Level Set Warning") {
        reset_warning_callback();
        
        ulog_status result = ulog_topic_level_set("test_topic", ULOG_LEVEL_INFO);
        
        CHECK(result == ULOG_STATUS_DISABLED);
        CHECK(warning_callback_count == 1);
        CHECK(strstr(warning_last_message, "ulog_topic_level_set") != nullptr);
        CHECK(strstr(warning_last_message, "ULOG_BUILD_TOPICS_NUM disabled") != nullptr);
    }
    
    SUBCASE("Topic Enable Warning") {
        reset_warning_callback();
        
        ulog_status result = ulog_topic_enable("test_topic");
        
        CHECK(result == ULOG_STATUS_DISABLED);
        CHECK(warning_callback_count == 1);
        CHECK(strstr(warning_last_message, "ulog_topic_enable") != nullptr);
        CHECK(strstr(warning_last_message, "ULOG_BUILD_TOPICS_NUM disabled") != nullptr);
    }
    
    SUBCASE("Topic Disable Warning") {
        reset_warning_callback();
        
        ulog_status result = ulog_topic_disable("test_topic");
        
        CHECK(result == ULOG_STATUS_DISABLED);
        CHECK(warning_callback_count == 1);
        CHECK(strstr(warning_last_message, "ulog_topic_disable") != nullptr);
        CHECK(strstr(warning_last_message, "ULOG_BUILD_TOPICS_NUM disabled") != nullptr);
    }
    
    SUBCASE("Topic Enable All Warning") {
        reset_warning_callback();
        
        ulog_status result = ulog_topic_enable_all();
        
        CHECK(result == ULOG_STATUS_DISABLED);
        CHECK(warning_callback_count == 1);
        CHECK(strstr(warning_last_message, "ulog_topic_enable_all") != nullptr);
        CHECK(strstr(warning_last_message, "ULOG_BUILD_TOPICS_NUM disabled") != nullptr);
    }
    
    SUBCASE("Topic Disable All Warning") {
        reset_warning_callback();
        
        ulog_status result = ulog_topic_disable_all();
        
        CHECK(result == ULOG_STATUS_DISABLED);
        CHECK(warning_callback_count == 1);
        CHECK(strstr(warning_last_message, "ulog_topic_disable_all") != nullptr);
        CHECK(strstr(warning_last_message, "ULOG_BUILD_TOPICS_NUM disabled") != nullptr);
    }
    
    SUBCASE("Topic Get ID Warning") {
        reset_warning_callback();
        
        ulog_topic_id result = ulog_topic_get_id("test_topic");
        
        CHECK(result == ULOG_TOPIC_ID_INVALID);
        CHECK(warning_callback_count == 1);
        CHECK(strstr(warning_last_message, "ulog_topic_get_id") != nullptr);
        CHECK(strstr(warning_last_message, "ULOG_BUILD_TOPICS_NUM disabled") != nullptr);
    }
    
    SUBCASE("Topic Add Warning") {
        reset_warning_callback();
        
        ulog_topic_id result = ulog_topic_add("test_topic", ULOG_OUTPUT_ALL, true);
        
        CHECK(result == ULOG_TOPIC_ID_INVALID);
        CHECK(warning_callback_count == 1);
        CHECK(strstr(warning_last_message, "ulog_topic_add") != nullptr);
        CHECK(strstr(warning_last_message, "ULOG_BUILD_TOPICS_NUM disabled") != nullptr);
    }
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Extra Output Functions Work In Base Config") {
    // In the base config (ULOG_CONFIG_BASE), extra outputs should be enabled
    // So these tests will verify the functions work when features are enabled
    
    SUBCASE("Output Add Function Available") {
        // This should work since ULOG_BUILD_EXTRA_OUTPUTS=8 in base config
        ulog_output_id result = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_TRACE);
        CHECK(result != ULOG_OUTPUT_INVALID);
        ulog_cleanup();
    }
    
    SUBCASE("Output Add File Function Available") {
        // This should work since ULOG_BUILD_EXTRA_OUTPUTS=8 in base config
        FILE *temp_file = tmpfile();
        REQUIRE(temp_file != nullptr);
        ulog_output_id result = ulog_output_add_file(temp_file, ULOG_LEVEL_TRACE);
        CHECK(result != ULOG_OUTPUT_INVALID);
        ulog_cleanup();
        fclose(temp_file);
    }
    
    SUBCASE("Output Remove Function Available") {
        // Add an output first
        ulog_output_id output_id = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_TRACE);
        REQUIRE(output_id != ULOG_OUTPUT_INVALID);
        
        // This should work since ULOG_BUILD_EXTRA_OUTPUTS=8 in base config
        ulog_status result = ulog_output_remove(output_id);
        CHECK(result == ULOG_STATUS_OK);
        ulog_cleanup();
    }
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Prefix Set Function Works In Base Config") {
    // In the base config, ULOG_BUILD_PREFIX_SIZE=16, so prefix functions should work
    
    // Define a dummy prefix function
    auto dummy_prefix_fn = [](ulog_event *ev, char *prefix, size_t prefix_size) {
        (void)ev; (void)prefix; (void)prefix_size;
    };
    
    // Call prefix set function (should work since ULOG_BUILD_PREFIX_SIZE=16 in base config)
    ulog_status result = ulog_prefix_set_fn(dummy_prefix_fn);
    
    CHECK(result == ULOG_STATUS_OK);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Warning Message Format") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call a disabled function
    ulog_color_config(true);
    
    // Verify warning message format: "'function_name' called with FEATURE disabled"
    CHECK(warning_callback_count == 1);
    CHECK(strstr(warning_last_message, "'ulog_color_config' called with ULOG_BUILD_COLOR disabled") != nullptr);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Multiple Warning Calls") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call multiple disabled functions
    ulog_color_config(true);
    ulog_time_config(false);
    ulog_topic_enable("test");
    
    // Should generate 3 warnings
    CHECK(warning_callback_count == 3);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "Warning Level Filtering") {
    // Add callback that only accepts ERROR level and above
    ulog_output_id error_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_ERROR);
    REQUIRE(error_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call disabled function (generates WARN level message)
    ulog_color_config(true);
    
    // Should not generate callback since WARN < ERROR
    CHECK(warning_callback_count == 0);
    
    ulog_cleanup();
}

TEST_CASE_FIXTURE(WarnNotEnabledTestFixture, "No Warnings When Feature Works") {
    ulog_output_id warning_output = ulog_output_add(warning_test_callback, nullptr, ULOG_LEVEL_WARN);
    REQUIRE(warning_output != ULOG_OUTPUT_INVALID);
    
    reset_warning_callback();
    
    // Call functions that should work (enabled features)
    ulog_output_level_set_all(ULOG_LEVEL_INFO);
    ulog_level_reset_levels();
    
    // Should not generate any warnings
    CHECK(warning_callback_count == 0);
    
    ulog_cleanup();
}
