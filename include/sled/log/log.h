/**
 * @file     : log
 * @created  : Thursday Feb 22, 2024 14:32:32 CST
 * @license  : MIT
 **/

#ifndef SLED_LOG_LOG_H
#define SLED_LOG_LOG_H
#pragma once
#include "sled/system/location.h"
#include <assert.h>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/compile.h>
#include <fmt/format.h>
#include <fmt/ostream.h>// support fmt base ostream
#include <fmt/printf.h>
#include <fmt/ranges.h>
#include <fmt/std.h>
#include <fmt/xchar.h>

namespace sled {
enum class LogLevel {
    kTrace = 0,
    kDebug = 1,
    kInfo = 2,
    kWarning = 3,
    kError = 4,
    kFatal = 5,
};
void SetLogLevel(LogLevel level);

void Log(LogLevel level, const char *tag, const char *fmt, const char *file_name, int line, const char *func_name, ...);

}// namespace sled

// #define _SLOG(level, tag, ...)                                                 \
//     sled::Log(level, tag, fmt, __FILE__, __FUNCTION__, __VA_ARGS__)

#define _SLOG(level, tag, fmt_str, ...)                                                                                \
    do {                                                                                                               \
        std::string __fmt_str;                                                                                         \
        try {                                                                                                          \
            __fmt_str = fmt::format(fmt_str, ##__VA_ARGS__);                                                           \
        } catch (const std::exception &e) {                                                                            \
            __fmt_str = " fmt error: " + std::string(e.what());                                                        \
        }                                                                                                              \
        sled::Log(level, tag, __fmt_str.c_str(), __FILE__, __LINE__, __FUNCTION__);                                    \
    } while (0)

#define SLOG(level, tag, fmt, ...) _SLOG(level, tag, fmt, ##__VA_ARGS__)
#define SLOG_TRACE(tag, fmt, ...) SLOG(sled::LogLevel::kTrace, tag, fmt, __VA_ARGS__)
#define SLOG_INFO(tag, fmt, ...) SLOG(sled::LogLevel::kInfo, tag, fmt, __VA_ARGS__)
#define SLOG_DEBUG(tag, fmt, ...) SLOG(sled::LogLevel::kDebug, tag, fmt, __VA_ARGS__)
#define SLOG_WARNING(tag, fmt, ...) SLOG(sled::LogLevel::kWarning, tag, fmt, __VA_ARGS__)
#define SLOG_ERROR(tag, fmt, ...) SLOG(sled::LogLevel::kError, tag, fmt, __VA_ARGS__)
#define SLOG_FATAL(tag, fmt, ...) SLOG(sled::LogLevel::kFatal, tag, fmt, __VA_ARGS__)

#define SLOG_IF(cond, level, tag, fmt, ...)                                                                            \
    do {                                                                                                               \
        if (cond) { SLOG(level, tag, fmt, __VA_ARGS__); }                                                              \
    } while (0)

#define SLOG_ASSERT(cond, tag, fmt, ...)                                                                               \
    do {                                                                                                               \
        if (!(cond)) {                                                                                                 \
            SLOG(sled::LogLevel::kFatal, tag, fmt, ##__VA_ARGS__);                                                     \
            assert(cond);                                                                                              \
        }                                                                                                              \
    } while (0)

#define LOGV_IF(cond, tag, fmt, ...) SLOG_IF(cond, sled::LogLevel::kTrace, tag, fmt, __VA_ARGS__)
#define LOGD_IF(cond, tag, fmt, ...) SLOG_IF(cond, sled::LogLevel::kDebug, tag, fmt, __VA_ARGS__)
#define LOGI_IF(cond, tag, fmt, ...) SLOG_IF(cond, sled::LogLevel::kInfo, tag, fmt, __VA_ARGS__)
#define LOGW_IF(cond, tag, fmt, ...) SLOG_IF(cond, sled::LogLevel::kWarning, tag, fmt, __VA_ARGS__)
#define LOGE_IF(cond, tag, fmt, ...) SLOG_IF(cond, sled::LogLevel::kError, tag, fmt, __VA_ARGS__)
#define LOGF_IF(cond, tag, fmt, ...) SLOG_IF(cond, sled::LogLevel::kFatal, tag, fmt, __VA_ARGS__)

#define CHECK(cond, fmt, ...) SLOG_ASSERT(cond, "DCHECK", fmt, ##__VA_ARGS__)
#define DCHECK(cond, fmt, ...) SLOG_ASSERT(cond, "DCHECK", fmt, ##__VA_ARGS__)

#define LOGV(tag, fmt, ...) SLOG(sled::LogLevel::kTrace, tag, fmt, ##__VA_ARGS__)
#define LOGD(tag, fmt, ...) SLOG(sled::LogLevel::kDebug, tag, fmt, ##__VA_ARGS__)
#define LOGI(tag, fmt, ...) SLOG(sled::LogLevel::kInfo, tag, fmt, ##__VA_ARGS__)
#define LOGW(tag, fmt, ...) SLOG(sled::LogLevel::kWarning, tag, fmt, ##__VA_ARGS__)
#define LOGE(tag, fmt, ...) SLOG(sled::LogLevel::kError, tag, fmt, ##__VA_ARGS__)
#define LOGF(tag, fmt, ...) SLOG(sled::LogLevel::kFatal, tag, fmt, ##__VA_ARGS__)

#define ASSERT(cond, fmt, ...) SLOG_ASSERT(cond, "ASSERT", fmt, ##__VA_ARGS__)

#define __LOG_EVERY_N(n, level, tag, fmt, ...)                                                                         \
    do {                                                                                                               \
        static int __sled_log_count##__FUNCTION__##__LINE__ = 0;                                                       \
        if (__sled_log_count##__FUNCTION__##__LINE__++ % n == 0) { SLOG(level, tag, fmt, ##__VA_ARGS__); }             \
    } while (0)

#define LOGV_EVERY_N(n, tag, fmt, ...) __LOG_EVERY_N(n, sled::LogLevel::kTrace, tag, fmt, ##__VA_ARGS__)
#define LOGD_EVERY_N(n, tag, fmt, ...) __LOG_EVERY_N(n, sled::LogLevel::kDebug, tag, fmt, ##__VA_ARGS__)
#define LOGI_EVERY_N(n, tag, fmt, ...) __LOG_EVERY_N(n, sled::LogLevel::kInfo, tag, fmt, ##__VA_ARGS__)
#define LOGW_EVERY_N(n, tag, fmt, ...) __LOG_EVERY_N(n, sled::LogLevel::kWarning, tag, fmt, ##__VA_ARGS__)
#define LOGE_EVERY_N(n, tag, fmt, ...) __LOG_EVERY_N(n, sled::LogLevel::kError, tag, fmt, ##__VA_ARGS__)
#define LOGF_EVERY_N(n, tag, fmt, ...) __LOG_EVERY_N(n, sled::LogLevel::kFatal, tag, fmt, ##__VA_ARGS__)

#define __LOG_ONCE(level, tag, fmt, ...)                                                                               \
    do {                                                                                                               \
        static bool __sled_log_once##__FUNCTION__##__LINE__ = false;                                                   \
        if (!__sled_log_once##__FUNCTION__##__LINE__) {                                                                \
            __sled_log_once##__FUNCTION__##__LINE__ = true;                                                            \
            SLOG(level, tag, fmt, ##__VA_ARGS__);                                                                      \
        }                                                                                                              \
    } while (0)

#define LOGV_ONCE(tag, fmt, ...) __LOG_ONCE(sled::LogLevel::kTrace, tag, fmt, ##__VA_ARGS__)
#define LOGD_ONCE(tag, fmt, ...) __LOG_ONCE(sled::LogLevel::kDebug, tag, fmt, ##__VA_ARGS__)
#define LOGI_ONCE(tag, fmt, ...) __LOG_ONCE(sled::LogLevel::kInfo, tag, fmt, ##__VA_ARGS__)
#define LOGW_ONCE(tag, fmt, ...) __LOG_ONCE(sled::LogLevel::kWarning, tag, fmt, ##__VA_ARGS__)
#define LOGE_ONCE(tag, fmt, ...) __LOG_ONCE(sled::LogLevel::kError, tag, fmt, ##__VA_ARGS__)
#define LOGF_ONCE(tag, fmt, ...) __LOG_ONCE(sled::LogLevel::kFatal, tag, fmt, ##__VA_ARGS__)

#endif// SLED_LOG_LOG_H
