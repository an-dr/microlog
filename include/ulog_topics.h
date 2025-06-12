#ifndef ULOG_TOPICS_H_
#define ULOG_TOPICS_H_

#include <stdbool.h> // For bool type
#include <stdint.h>  // For uint8_t

// Forward declaration for ulog_Event, if needed by any function prototypes here
// However, it's better if ulog_topics.h doesn't need full ulog_Event details.
// If ulog_Event is only used by ulog.c, then this is not needed here.

#if FEATURE_TOPICS

#define TOPIC_NOT_FOUND (-1)

/**
 * @brief Structure to hold topic information.
 */
struct Topic {
    const char *name; /**< Name of the topic. */
    uint8_t level;    /**< Current log level for this topic. */
    bool enabled;     /**< Whether this topic is enabled or disabled. */
};

/*
 * Public API function declarations for topics
 */

/**
 * @brief Sets the log level for a specific topic.
 *
 * @param topic_name Name of the topic.
 * @param level New log level for the topic.
 */
void ulog_set_topic_level(const char *topic_name, uint8_t level);

/**
 * @brief Gets the log level for a specific topic.
 *
 * @param topic_name Name of the topic.
 * @return Current log level for the topic, or default level if topic not found.
 */
uint8_t ulog_get_topic_level(const char *topic_name);

/**
 * @brief Enables logging for a specific topic.
 *
 * @param topic_name Name of the topic.
 */
void ulog_enable_topic(const char *topic_name);

/**
 * @brief Disables logging for a specific topic.
 *
 * @param topic_name Name of the topic.
 */
void ulog_disable_topic(const char *topic_name);

/**
 * @brief Enables logging for all topics.
 */
void ulog_enable_all_topics(void);

/**
 * @brief Disables logging for all topics.
 */
void ulog_disable_all_topics(void);

/**
 * @brief Gets the ID of a topic.
 *
 * @param topic_name Name of the topic.
 * @return ID of the topic, or TOPIC_NOT_FOUND if not found.
 */
int ulog_get_topic_id(const char *topic_name);


/*
 * Functions that were static in ulog.c but need to be exposed for ulog.c
 * or are part of the implementation of public topic functions.
 */

/**
 * @brief Sets the log level for a topic by its ID.
 *
 * @param topic_id ID of the topic.
 * @param level New log level for the topic.
 */
void _ulog_set_topic_level(int topic_id, uint8_t level);

/**
 * @brief Gets the log level for a topic by its ID.
 *
 * @param topic_id ID of the topic.
 * @return Current log level for the topic.
 */
uint8_t _ulog_get_topic_level(int topic_id);

/**
 * @brief Enables a topic by its ID.
 *
 * @param topic_id ID of the topic.
 */
void _ulog_enable_topic(int topic_id);

/**
 * @brief Disables a topic by its ID.
 *
 * @param topic_id ID of the topic.
 */
void _ulog_disable_topic(int topic_id);

/**
 * @brief Checks if a topic is enabled by its ID.
 *
 * @param topic_id ID of the topic.
 * @return True if the topic is enabled, false otherwise.
 */
bool is_topic_enabled(int topic_id);

/**
 * @brief Prints the topic name if it exists.
 * Used by write_formatted_message in ulog.c
 *
 * @param buffer The buffer to print to.
 * @param size Size of the buffer.
 * @param topic_id ID of the topic.
 * @return Number of characters printed.
 */
int print_topic(char *buffer, size_t size, int topic_id);

#endif /* FEATURE_TOPICS */

#endif /* ULOG_TOPICS_H_ */
