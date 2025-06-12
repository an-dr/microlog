#ifndef ULOG_TOPICS_H
#define ULOG_TOPICS_H

#include "ulog_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TOPIC_NOT_FOUND 0x7FFFFFFF
#define logt_trace(TOPIC_NAME, ...) ulog_log(LOG_TRACE, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_debug(TOPIC_NAME, ...) ulog_log(LOG_DEBUG, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_info(TOPIC_NAME, ...) ulog_log(LOG_INFO, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_warn(TOPIC_NAME, ...) ulog_log(LOG_WARN, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_error(TOPIC_NAME, ...) ulog_log(LOG_ERROR, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_fatal(TOPIC_NAME, ...) ulog_log(LOG_FATAL, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

#if FEATURE_TOPICS
int ulog_add_topic(const char *topic_name, bool enable);
int ulog_set_topic_level(const char *topic_name, int level);
int ulog_get_topic_id(const char *topic_name);
int ulog_enable_topic(const char *topic_name);
int ulog_disable_topic(const char *topic_name);
int ulog_enable_all_topics(void);
int ulog_disable_all_topics(void);
#endif

#ifdef __cplusplus
}
#endif

#endif // ULOG_TOPICS_H
