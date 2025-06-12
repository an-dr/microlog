#include "ulog.h" // For ulog_Event, level_strings, LOG_TRACE, FEATURE_TOPICS, CFG_TOPICS_DINAMIC_ALLOC etc.
#include "ulog_topics.h"

#if FEATURE_TOPICS

#include <stdio.h>
#include <string.h>
#include <stdlib.h> // For malloc/free if CFG_TOPICS_DINAMIC_ALLOC

/*
 * Static variables and functions related to topics
 */

#if !CFG_TOPICS_DINAMIC_ALLOC
static struct Topic topics[CFG_MAX_TOPICS];
static uint8_t topic_count = 0;
#else
static struct Topic *topics = NULL;
static uint8_t topic_count = 0;
static uint8_t topic_capacity = 0;
#endif

static bool new_topic_enabled = true; // Default behavior for new topics

/**
 * @brief Gets a pointer to the beginning of the topics array.
 *
 * @return Pointer to the first topic, or NULL if no topics.
 */
static struct Topic* _get_topic_begin(void) {
    #if !CFG_TOPICS_DINAMIC_ALLOC
    return topics;
    #else
    return topics; // topics itself is the pointer
    #endif
}

/**
 * @brief Gets a pointer to the next topic in the array.
 *
 * @param current_topic Pointer to the current topic.
 * @return Pointer to the next topic, or NULL if at the end.
 */
static struct Topic* _get_topic_next(struct Topic* current_topic) {
    if (!current_topic) return NULL;
    // Check if current_topic is within the bounds of the array
    if (current_topic < &topics[0] || current_topic >= &topics[topic_count]) {
        return NULL; // Should not happen if used correctly
    }
    if (current_topic == &topics[topic_count - 1]) {
        return NULL; // Last topic
    }
    return current_topic + 1;
}

/**
 * @brief Gets a pointer to a topic by its ID.
 *
 * @param topic_id ID of the topic.
 * @return Pointer to the topic, or NULL if ID is invalid.
 */
static struct Topic* _get_topic_ptr(int topic_id) {
    if (topic_id < 0 || topic_id >= topic_count) {
        return NULL;
    }
    return &topics[topic_id];
}

#if CFG_TOPICS_DINAMIC_ALLOC
/**
 * @brief Creates a new topic or returns an existing one. (Dynamic allocation version)
 *
 * @param topic_name Name of the topic.
 * @return ID of the topic, or TOPIC_NOT_FOUND on failure.
 */
static int _create_topic(const char *topic_name) {
    // Check if topic already exists
    for (uint8_t i = 0; i < topic_count; i++) {
        if (strcmp(topics[i].name, topic_name) == 0) {
            return i; // Topic found
        }
    }

    // Topic not found, create new one
    if (topic_count >= topic_capacity) {
        // Resize topics array
        uint8_t new_capacity = topic_capacity == 0 ? CFG_MAX_TOPICS : topic_capacity * 2;
        if (new_capacity == 0) new_capacity = 4; // Initial capacity if CFG_MAX_TOPICS is 0 (though unlikely for dynamic)
        struct Topic *new_topics = (struct Topic *)realloc(topics, new_capacity * sizeof(struct Topic));
        if (!new_topics) {
            // Allocation failed
            return TOPIC_NOT_FOUND;
        }
        topics = new_topics;
        topic_capacity = new_capacity;
    }

    // Check again if we have space (realloc might fail to give more capacity than current count)
    if (topic_count >= topic_capacity) {
         return TOPIC_NOT_FOUND; // Should not happen if realloc succeeded and new_capacity > topic_count
    }

    topics[topic_count].name = topic_name; // Note: This assumes topic_name is persistent (e.g. string literal)
    topics[topic_count].level = global_log_level; // Default to global log level
    topics[topic_count].enabled = new_topic_enabled;
    return topic_count++;
}
#else // !CFG_TOPICS_DINAMIC_ALLOC
/**
 * @brief Creates a new topic or returns an existing one. (Static allocation version)
 *
 * @param topic_name Name of the topic.
 * @return ID of the topic, or TOPIC_NOT_FOUND if no space.
 */
static int _create_topic(const char *topic_name) {
    // Check if topic already exists
    for (uint8_t i = 0; i < topic_count; i++) {
        if (strcmp(topics[i].name, topic_name) == 0) {
            return i; // Topic found
        }
    }

    // Topic not found, create new one if space available
    if (topic_count < CFG_MAX_TOPICS) {
        topics[topic_count].name = topic_name; // Note: This assumes topic_name is persistent
        topics[topic_count].level = global_log_level; // Default to global log level
        topics[topic_count].enabled = new_topic_enabled;
        return topic_count++;
    }
    return TOPIC_NOT_FOUND; // No space for new topic
}
#endif


/*
 * Public API function definitions (from ulog_topics.h)
 */

void ulog_set_topic_level(const char *topic_name, uint8_t level) {
    int topic_id = ulog_get_topic_id(topic_name);
    if (topic_id != TOPIC_NOT_FOUND) {
        _ulog_set_topic_level(topic_id, level);
    }
}

uint8_t ulog_get_topic_level(const char *topic_name) {
    int topic_id = ulog_get_topic_id(topic_name);
    if (topic_id != TOPIC_NOT_FOUND) {
        return _ulog_get_topic_level(topic_id);
    }
    return global_log_level; // Or some other default, like ULOG_LEVEL_NONE or an error indicator?
                             // Matching original behavior: returns global_log_level if topic not found by ulog_log
}

void ulog_enable_topic(const char *topic_name) {
    int topic_id = ulog_get_topic_id(topic_name);
    if (topic_id != TOPIC_NOT_FOUND) {
        _ulog_enable_topic(topic_id);
    }
}

void ulog_disable_topic(const char *topic_name) {
    int topic_id = ulog_get_topic_id(topic_name);
    if (topic_id != TOPIC_NOT_FOUND) {
        _ulog_disable_topic(topic_id);
    }
}

void ulog_enable_all_topics(void) {
    new_topic_enabled = true;
    for (uint8_t i = 0; i < topic_count; i++) {
        topics[i].enabled = true;
    }
}

void ulog_disable_all_topics(void) {
    new_topic_enabled = false;
    for (uint8_t i = 0; i < topic_count; i++) {
        topics[i].enabled = false;
    }
}

int ulog_get_topic_id(const char *topic_name) {
    if (!topic_name) return TOPIC_NOT_FOUND; // Handle NULL topic name gracefully

    for (uint8_t i = 0; i < topic_count; i++) {
        if (strcmp(topics[i].name, topic_name) == 0) {
            return i;
        }
    }
    // If dynamic allocation and auto-creation is desired here:
    #if CFG_TOPICS_AUTO_CREATE_ON_GET_ID && CFG_TOPICS_DINAMIC_ALLOC
    return _create_topic(topic_name);
    #elif CFG_TOPICS_AUTO_CREATE_ON_GET_ID // Static allocation
    if (topic_count < CFG_MAX_TOPICS) {
        return _create_topic(topic_name);
    }
    #endif
    return TOPIC_NOT_FOUND;
}


/*
 * Non-static versions of formerly static functions, now defined in ulog_topics.h
 */

void _ulog_set_topic_level(int topic_id, uint8_t level) {
    struct Topic *topic = _get_topic_ptr(topic_id);
    if (topic) {
        topic->level = level;
    }
}

uint8_t _ulog_get_topic_level(int topic_id) {
    struct Topic *topic = _get_topic_ptr(topic_id);
    if (topic) {
        return topic->level;
    }
    // This case should ideally be handled by the caller,
    // or ulog_log's logic which uses global_log_level if topic_id is invalid.
    // For safety, returning a default or error. Original code implies global_log_level is used.
    return global_log_level;
}

void _ulog_enable_topic(int topic_id) {
    struct Topic *topic = _get_topic_ptr(topic_id);
    if (topic) {
        topic->enabled = true;
    }
}

void _ulog_disable_topic(int topic_id) {
    struct Topic *topic = _get_topic_ptr(topic_id);
    if (topic) {
        topic->enabled = false;
    }
}

bool is_topic_enabled(int topic_id) {
    struct Topic *topic = _get_topic_ptr(topic_id);
    if (topic) {
        return topic->enabled;
    }
    return new_topic_enabled; // Default for non-existing topics, or should be false?
                              // Original ulog_log implies that if topic_id is TOPIC_NOT_FOUND,
                              // it uses global settings, not topic specific.
                              // If topic_id is valid but somehow points to an uninitialized topic,
                              // it's safer to return false or a default.
                              // The current logic of ulog_log: if topic_id == TOPIC_NOT_FOUND, it checks global_log_level.
                              // if topic_id is valid, it then checks topic->enabled.
                              // So this function should ideally only be called with valid topic_id.
                              // Let's ensure it returns false for invalid topic_id to be safe.
    if (topic_id < 0 || topic_id >= topic_count) return false; // Explicitly handle invalid ID

    // Fallback, though _get_topic_ptr should have returned NULL for invalid topic_id
    return new_topic_enabled;
}

int print_topic(char *buffer, size_t size, int topic_id) {
    struct Topic *topic = _get_topic_ptr(topic_id);
    if (topic && topic->name) {
        return snprintf(buffer, size, "[%s] ", topic->name);
    }
    return 0;
}

#endif /* FEATURE_TOPICS */
