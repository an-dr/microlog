#ifndef ULOG_OUTPUT_H
#define ULOG_OUTPUT_H

#include "ulog_core.h"

#ifdef __cplusplus
extern "C" {
#endif

#if FEATURE_EXTRA_OUTPUTS
/// @brief Adds a callback
int ulog_add_callback(ulog_LogFn function, void *arg, int level);

/// @brief Add file callback
int ulog_add_fp(FILE *fp, int level);
#endif

#ifdef __cplusplus
}
#endif

#endif // ULOG_OUTPUT_H
