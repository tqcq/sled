/**
 * @file     : log
 * @created  : Thursday Feb 22, 2024 14:32:32 CST
 * @license  : MIT
 **/

#ifndef LOG_H
#define LOG_H
#include <fmt/format.h>

namespace sled {
enum class LogLevel {
    kTrace,
    kDebug,
    kInfo,
    kWarning,
    kError,
    kFatal,
};

void Log(LogLevel level,
         const char *tag,
         const char *fmt,
         const char *file_name,
         int line,
         const char *func_name,
         ...);

}// namespace sled

// #define _SLOG(level, tag, ...)                                                 \
//     sled::Log(level, tag, fmt, __FILE__, __FUNCTION__, __VA_ARGS__)

#define _SLOG(level, tag, fmt_str, ...)                                        \
    sled::Log(level, tag, fmt::format(fmt_str, #__VA_ARGS__).c_str(),          \
              __FILE__, __LINE__, __FUNCTION__)

#define SLOG(level, tag, fmt, ...) _SLOG(level, tag, fmt, #__VA_ARGS__)
#define SLOG_TRACE(tag, fmt, ...)                                              \
    SLOG(sled::LogLevel::kTrace, tag, fmt, __VA_ARGS__)
#define SLOG_INFO(tag, fmt, ...)                                               \
    SLOG(sled::LogLevel::kInfo, tag, fmt, __VA_ARGS__)
#define SLOG_DEBUG(tag, fmt, ...)                                              \
    SLOG(sled::LogLevel::kDebug, tag, fmt, __VA_ARGS__)
#define SLOG_WARNING(tag, fmt, ...)                                            \
    SLOG(sled::LogLevel::kWarning, tag, fmt, __VA_ARGS__)
#define SLOG_ERROR(tag, fmt, ...)                                              \
    SLOG(sled::LogLevel::kError, tag, fmt, __VA_ARGS__)
#define SLOG_FATAL(tag, fmt, ...)                                              \
    SLOG(sled::LogLevel::kFatal, tag, fmt, __VA_ARGS__)

#define SLOG_IF(cond, level, tag, fmt, ...)                                    \
    do {                                                                       \
        if (cond) { SLOG(level, tag, fmt, __VA_ARGS__); }                      \
    } while (0)

#define SLOG_ASSERT(cond, tag, fmt, ...)                                       \
    do {                                                                       \
        if (!(cond)) {                                                         \
            SLOG(sled::LogLevel::kFatal, __VA_ARGS__);                         \
            assert(cond);                                                      \
        }                                                                      \
    } while (0)

#define LOGV_IF(cond, tag, fmt, ...)                                           \
    SLOG_IF(cond, sled::LogLevel::kTrace, tag, fmt, __VA_ARGS__)
#define LOGD_IF(cond, tag, fmt, ...)                                           \
    SLOG_IF(cond, sled::LogLevel::kDebug, tag, fmt, __VA_ARGS__)
#define LOGI_IF(cond, tag, fmt, ...)                                           \
    SLOG_IF(cond, sled::LogLevel::kInfo, tag, fmt, __VA_ARGS__)
#define LOGW_IF(cond, tag, fmt, ...)                                           \
    SLOG_IF(cond, sled::LogLevel::kWarning, tag, fmt, __VA_ARGS__)
#define LOGE_IF(cond, tag, fmt, ...)                                           \
    SLOG_IF(cond, sled::LogLevel::kError, tag, fmt, __VA_ARGS__)
#define LOGF_IF(cond, tag, fmt, ...)                                           \
    SLOG_IF(cond, sled::LogLevel::kFatal, tag, fmt, __VA_ARGS__)

#define LOGV(tag, fmt, ...) SLOG(sled::LogLevel::kTrace, tag, fmt, #__VA_ARGS__)
#define LOGD(tag, fmt, ...) SLOG(sled::LogLevel::kDebug, tag, fmt, #__VA_ARGS__)
#define LOGI(tag, fmt, ...) SLOG(sled::LogLevel::kInfo, tag, fmt, #__VA_ARGS__)
#define LOGW(tag, fmt, ...)                                                    \
    SLOG(sled::LogLevel::kWarning, tag, fmt, #__VA_ARGS__)
#define LOGE(tag, fmt, ...) SLOG(sled::LogLevel::kError, tag, fmt, #__VA_ARGS__)
#define LOGF(tag, fmt, ...) SLOG(sled::LogLevel::kFatal, tag, fmt, #__VA_ARGS__)

#endif// LOG_H
