// *************************************************************************
//
// Copyright (c) 2024 Andrei Gramakov. All rights reserved.
//
// This file is licensed under the terms of the MIT license.
// For a copy, see: https://opensource.org/licenses/MIT
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#pragma once

inline void ilog_trace(const char *fmt, ...);
inline void ilog_debug(const char *fmt, ...);
inline void ilog_info(const char *fmt, ...);
inline void ilog_warn(const char *fmt, ...);
inline void ilog_error(const char *fmt, ...);
inline void ilog_fatal(const char *fmt, ...);

__attribute__((weak)) void ilog_trace(const char *fmt, ...){}
__attribute__((weak)) void ilog_debug(const char *fmt, ...){}
__attribute__((weak)) void ilog_info(const char *fmt, ...){}
__attribute__((weak)) void ilog_warn(const char *fmt, ...){}
__attribute__((weak)) void ilog_error(const char *fmt, ...){}
__attribute__((weak)) void ilog_fatal(const char *fmt, ...){}

