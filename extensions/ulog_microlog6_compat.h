// *************************************************************************
//
// ulog microlog6 compatibility extension
// https://github.com/an-dr/microlog
//
// *************************************************************************
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// This file is licensed under the terms of the MIT license.
// For a copy, see: https://opensource.org/licenses/MIT
//
// *************************************************************************

#pragma once

#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
   microlog6 Compatibility Layer

   This extension provides backward compatibility for code written against
   microlog v6.x API. It maps the old API to the new v7.x API.

   Usage:
       #include "ulog.h"
       #include "extensions/ulog_microlog6_compat.h"

   Note: Build-time configuration macros need manual mapping:
       - ULOG_NO_COLOR          → ULOG_BUILD_COLOR=0
       - ULOG_CUSTOM_PREFIX_SIZE → ULOG_BUILD_PREFIX_SIZE
       - ULOG_HAVE_TIME         → ULOG_BUILD_TIME=1
       - ULOG_USE_EMOJI         → (not supported in v7.x)
       - ULOG_EXTRA_OUTPUTS     → ULOG_BUILD_EXTRA_OUTPUTS
       - ULOG_HIDE_FILE_STRING  → ULOG_BUILD_SOURCE_LOCATION=0
       - ULOG_SHORT_LEVEL_STRINGS → ULOG_BUILD_LEVEL_SHORT=1
       - ULOG_TOPICS_NUM (static) → ULOG_BUILD_TOPICS_MODE=1 + ULOG_BUILD_TOPICS_STATIC_NUM
       - ULOG_TOPICS_NUM=-1       → ULOG_BUILD_TOPICS_MODE=2
       - ULOG_RUNTIME_MODE        → ULOG_BUILD_DYNAMIC_CONFIG=1
============================================================================ */

/* ============================================================================
   Core: Log Level Enum Compatibility
============================================================================ */

// Map old log level enum to new ones
enum {
    LOG_TRACE = ULOG_LEVEL_TRACE,
    LOG_DEBUG = ULOG_LEVEL_DEBUG,
    LOG_INFO  = ULOG_LEVEL_INFO,
    LOG_WARN  = ULOG_LEVEL_WARN,
    LOG_ERROR = ULOG_LEVEL_ERROR,
    LOG_FATAL = ULOG_LEVEL_FATAL
};

/* ============================================================================
   Core: Basic Logging Macros
============================================================================ */

#define log_trace(...) ulog_trace(__VA_ARGS__)
#define log_debug(...) ulog_debug(__VA_ARGS__)
#define log_info(...)  ulog_info(__VA_ARGS__)
#define log_warn(...)  ulog_warn(__VA_ARGS__)
#define log_error(...) ulog_error(__VA_ARGS__)
#define log_fatal(...) ulog_fatal(__VA_ARGS__)

/* ============================================================================
   Core: Event Type Compatibility
============================================================================ */

// Type aliases for backward compatibility
typedef ulog_event ulog_Event;  // Note: capital E in old version
typedef ulog_output_handler_fn ulog_LogFn;

/* ============================================================================
   Core: Functions with Compatible Signatures
============================================================================ */

/// @brief Returns the string representation of the level (compatible)
static inline const char *ulog_get_level_string(int level) {
    return ulog_level_to_string((ulog_level)level);
}

/// @brief Sets the debug level for all outputs
static inline void ulog_set_level(int level) {
    (void)ulog_output_level_set_all((ulog_level)level);
}

/// @brief Disables/enables logging to stdout
/// @param enable true = quiet mode (disable stdout), false = enable stdout
static inline void ulog_set_quiet(bool enable) {
    if (enable) {
        // Set stdout to a level higher than FATAL to effectively disable it
        (void)ulog_output_level_set(ULOG_OUTPUT_STDOUT,
                                    (ulog_level)(ULOG_LEVEL_FATAL + 1));
    } else {
        // Re-enable stdout at TRACE level
        (void)ulog_output_level_set(ULOG_OUTPUT_STDOUT, ULOG_LEVEL_TRACE);
    }
}

/* ============================================================================
   Feature: Lock Compatibility
============================================================================ */

typedef void (*ulog_LockFn)(bool lock, void *lock_arg);

// Internal static storage for lock wrapper
static ulog_LockFn _ulog_compat_stored_lock_fn = NULL;
static void *_ulog_compat_stored_lock_arg = NULL;

// Internal wrapper function to adapt old void return to new ulog_status return
static inline ulog_status _ulog_compat_lock_wrapper(bool lock, void *arg) {
    (void)arg;  // Use stored args instead
    if (_ulog_compat_stored_lock_fn != NULL) {
        _ulog_compat_stored_lock_fn(lock, _ulog_compat_stored_lock_arg);
    }
    return ULOG_STATUS_OK;
}

/// @brief Sets the lock function (compatible wrapper)
/// Note: This uses static storage and may not work with multiple different locks
/// For production use with multiple locks, consider a more sophisticated approach
static inline void ulog_set_lock(ulog_LockFn function, void *lock_arg) {
    if (function == NULL) {
        _ulog_compat_stored_lock_fn = NULL;
        _ulog_compat_stored_lock_arg = NULL;
        ulog_lock_set_fn(NULL, NULL);
        return;
    }

    _ulog_compat_stored_lock_fn = function;
    _ulog_compat_stored_lock_arg = lock_arg;
    ulog_lock_set_fn(_ulog_compat_lock_wrapper, NULL);
}

/* ============================================================================
   Feature: Runtime Config Compatibility
============================================================================ */

#if defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1

static inline void ulog_configure_color(bool enabled) {
    (void)ulog_color_config(enabled);
}

static inline void ulog_configure_prefix(bool enabled) {
    (void)ulog_prefix_config(enabled);
}

static inline void ulog_configure_file_string(bool enabled) {
    (void)ulog_source_location_config(enabled);
}

static inline void ulog_configure_time(bool enabled) {
    (void)ulog_time_config(enabled);
}

static inline void ulog_configure_levels(bool use_short_levels) {
    if (use_short_levels) {
        (void)ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_SHORT);
    } else {
        (void)ulog_level_config(ULOG_LEVEL_CONFIG_STYLE_DEFAULT);
    }
}

static inline void ulog_configure_topics(bool enabled) {
    (void)ulog_topic_config(enabled);
}

#endif  // ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Feature: Custom Prefix Compatibility
============================================================================ */

#if (defined(ULOG_BUILD_PREFIX_SIZE) && ULOG_BUILD_PREFIX_SIZE > 0) || \
    (defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1)

typedef void (*ulog_PrefixFn)(ulog_Event *ev, char *prefix, size_t prefix_size);

/// @brief Sets the prefix function (compatible wrapper)
static inline void ulog_set_prefix_fn(ulog_PrefixFn function) {
    // The signatures are compatible, just cast
    (void)ulog_prefix_set_fn((ulog_prefix_fn)function);
}

#endif  // ULOG_BUILD_PREFIX_SIZE || ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Feature: Extra Outputs Compatibility
============================================================================ */

#if (defined(ULOG_BUILD_EXTRA_OUTPUTS) && ULOG_BUILD_EXTRA_OUTPUTS > 0) || \
    (defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1)

/// @brief Adds a callback (returns int for compatibility)
/// @return 0 if success, -1 if failed
static inline int ulog_add_callback(ulog_LogFn function, void *arg, int level) {
    ulog_output_id id = ulog_output_add(function, arg, (ulog_level)level);
    return (id == ULOG_OUTPUT_INVALID) ? -1 : 0;
}

/// @brief Add file callback (returns int for compatibility)
/// @return 0 if success, -1 if failed
static inline int ulog_add_fp(FILE *fp, int level) {
    ulog_output_id id = ulog_output_add_file(fp, (ulog_level)level);
    return (id == ULOG_OUTPUT_INVALID) ? -1 : 0;
}

#endif  // ULOG_BUILD_EXTRA_OUTPUTS || ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Feature: Topics Compatibility
============================================================================ */

#if (defined(ULOG_BUILD_TOPICS_MODE) && ULOG_BUILD_TOPICS_MODE != 0) || \
    (defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1)

// Topic macros using old naming convention
#define TOPIC_NOT_FOUND ULOG_TOPIC_ID_INVALID

#define logt_trace(TOPIC_NAME, ...) ulog_topic_trace(TOPIC_NAME, __VA_ARGS__)
#define logt_debug(TOPIC_NAME, ...) ulog_topic_debug(TOPIC_NAME, __VA_ARGS__)
#define logt_info(TOPIC_NAME, ...)  ulog_topic_info(TOPIC_NAME, __VA_ARGS__)
#define logt_warn(TOPIC_NAME, ...)  ulog_topic_warn(TOPIC_NAME, __VA_ARGS__)
#define logt_error(TOPIC_NAME, ...) ulog_topic_error(TOPIC_NAME, __VA_ARGS__)
#define logt_fatal(TOPIC_NAME, ...) ulog_topic_fatal(TOPIC_NAME, __VA_ARGS__)

/// @brief Adds a topic (compatible with old API)
/// @param topic_name Topic name
/// @param enable true to enable topic, false to disable
/// @return Topic ID if success, -1 if failed
static inline int ulog_add_topic(const char *topic_name, bool enable) {
    // In v6, enable meant setting level to TRACE (enabled) or a high level (disabled)
    ulog_level level = enable ? ULOG_LEVEL_TRACE : (ulog_level)(ULOG_LEVEL_FATAL + 1);
    ulog_topic_id id = ulog_topic_add(topic_name, ULOG_OUTPUT_ALL, level);
    return (id == ULOG_TOPIC_ID_INVALID) ? -1 : (int)id;
}

/// @brief Sets the debug level of a given topic (returns int for compatibility)
/// @return 0 if success, -1 if failed
static inline int ulog_set_topic_level(const char *topic_name, int level) {
    ulog_status status = ulog_topic_level_set(topic_name, (ulog_level)level);
    return (status == ULOG_STATUS_OK) ? 0 : -1;
}

/// @brief Gets the topic ID (compatible)
/// @return Topic ID if success, -1 if failed, TOPIC_NOT_FOUND if not found
static inline int ulog_get_topic_id(const char *topic_name) {
    return (int)ulog_topic_get_id(topic_name);
}

/// @brief Enables the topic (sets level to TRACE)
/// @return 0 if success, -1 if failed
static inline int ulog_enable_topic(const char *topic_name) {
    ulog_status status = ulog_topic_level_set(topic_name, ULOG_LEVEL_TRACE);
    return (status == ULOG_STATUS_OK) ? 0 : -1;
}

/// @brief Disables the topic (sets level higher than FATAL)
/// @return 0 if success, -1 if failed
static inline int ulog_disable_topic(const char *topic_name) {
    ulog_status status = ulog_topic_level_set(topic_name,
                                              (ulog_level)(ULOG_LEVEL_FATAL + 1));
    return (status == ULOG_STATUS_OK) ? 0 : -1;
}

/// @brief Enables all topics
/// Note: In v7.x, there's no direct API for this. This is a simplified version
/// that only works if you track your topics externally
/// @return 0 (always succeeds in this simple implementation)
static inline int ulog_enable_all_topics(void) {
    // v7.x doesn't provide an iterate-all-topics API
    // Users must track topics if they need this functionality
    // This is a limitation of the compatibility layer
    return 0;  // Return success but note this is incomplete
}

/// @brief Disables all topics
/// Note: Same limitation as ulog_enable_all_topics
/// @return 0 (always succeeds in this simple implementation)
static inline int ulog_disable_all_topics(void) {
    // Same limitation as enable_all_topics
    return 0;  // Return success but note this is incomplete
}

#endif  // ULOG_BUILD_TOPICS_MODE || ULOG_BUILD_DYNAMIC_CONFIG

/* ============================================================================
   Feature Flags Compatibility (read-only)
============================================================================ */

// Map old feature flags to new build configuration checks
#if defined(ULOG_BUILD_COLOR) && ULOG_BUILD_COLOR == 1
    #define ULOG_FEATURE_COLOR true
#else
    #define ULOG_FEATURE_COLOR false
#endif

#if defined(ULOG_BUILD_PREFIX_SIZE) && ULOG_BUILD_PREFIX_SIZE > 0
    #define ULOG_FEATURE_CUSTOM_PREFIX true
#else
    #define ULOG_FEATURE_CUSTOM_PREFIX false
#endif

#if defined(ULOG_BUILD_TIME) && ULOG_BUILD_TIME == 1
    #define ULOG_FEATURE_TIME true
#else
    #define ULOG_FEATURE_TIME false
#endif

#if defined(ULOG_BUILD_EXTRA_OUTPUTS) && ULOG_BUILD_EXTRA_OUTPUTS > 0
    #define ULOG_FEATURE_EXTRA_OUTPUTS true
#else
    #define ULOG_FEATURE_EXTRA_OUTPUTS false
#endif

#if defined(ULOG_BUILD_SOURCE_LOCATION) && ULOG_BUILD_SOURCE_LOCATION == 1
    #define ULOG_FEATURE_FILE_STRING true
#elif defined(ULOG_BUILD_SOURCE_LOCATION) && ULOG_BUILD_SOURCE_LOCATION == 0
    #define ULOG_FEATURE_FILE_STRING false
#else
    #define ULOG_FEATURE_FILE_STRING true  // Default is enabled in v7.x
#endif

#if defined(ULOG_BUILD_LEVEL_SHORT) && ULOG_BUILD_LEVEL_SHORT == 1
    #define ULOG_FEATURE_SHORT_LEVELS true
#else
    #define ULOG_FEATURE_SHORT_LEVELS false
#endif

#if defined(ULOG_BUILD_TOPICS_MODE) && ULOG_BUILD_TOPICS_MODE != 0
    #define ULOG_FEATURE_TOPICS true
#else
    #define ULOG_FEATURE_TOPICS false
#endif

#if defined(ULOG_BUILD_DYNAMIC_CONFIG) && ULOG_BUILD_DYNAMIC_CONFIG == 1
    #define ULOG_FEATURE_RUNTIME_MODE true
#else
    #define ULOG_FEATURE_RUNTIME_MODE false
#endif

// Note: ULOG_FEATURE_EMOJI_LEVELS is not supported in v7.x
#define ULOG_FEATURE_EMOJI_LEVELS false

#ifdef __cplusplus
}
#endif
