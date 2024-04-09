#include "sled/log/log.h"
#include "sled/synchronization/mutex.h"
#include "sled/time_utils.h"
#include <atomic>
#include <ctime>
#include <fmt/format.h>
#include <fstream>
#include <iostream>

namespace sled {

static const char *
GetConsoleColorPrefix(LogLevel level)
{
    switch (level) {
    case LogLevel::kTrace:
        return "\033[0;37m";
    case LogLevel::kDebug:
        return "\033[0;36m";
    case LogLevel::kInfo:
        return "\033[0m";
        // return "\033[0;32m";
    case LogLevel::kWarning:
        return "\033[0;33m";
    case LogLevel::kError:
        return "\033[0;31m";
    case LogLevel::kFatal:
        return "\033[0;31m";
    default:
        return "\033[0m";
    }
}

static const char *
GetConsoleColorSuffix()
{
    return "\033[0m";
}

static const char *
ToShortString(LogLevel level)
{
    switch (level) {
    case LogLevel::kTrace:
        return "T";
    case LogLevel::kDebug:
        return "D";
    case LogLevel::kInfo:
        return "I";
    case LogLevel::kWarning:
        return "W";
    case LogLevel::kError:
        return "E";
    case LogLevel::kFatal:
        return "F";
    default:
        return "#";
    }
}

class ScopedAtomicWaiter {
public:
    ScopedAtomicWaiter(std::atomic_bool &flag) : flag_(flag)
    {
        bool old = true;
        while (!flag_.compare_exchange_weak(old, false)) { continue; }
    }

    ~ScopedAtomicWaiter() { flag_.store(true); }

private:
    std::atomic_bool &flag_;
};

static std::string
GetCurrentUTCTime()
{
#if true
    // struct timespec tp;
    // clock_gettime(CLOCK_REALTIME, &tp);
    // int64_t now = tp.tv_sec * kNumNanosecsPerSec + tp.tv_nsec;
    // std::time_t t = tp.tv_sec;
    const int64_t now = TimeUTCNanos();
    std::time_t t     = now / kNumNanosecsPerSec;
#else
    std::time_t t = std::time(nullptr);
#endif
    std::tm tm = *std::gmtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    std::string result(buf);
#if true
    return result + fmt::format(".{:03d}Z", static_cast<int>(now % kNumNanosecsPerSec) / kNumNanosecsPerMillisec);
#else
    return result + "Z";
#endif
    return result;
}

static LogLevel g_log_level = LogLevel::kTrace;
static std::string g_log_file_name;
static std::ofstream g_log_stream;

void
SetLogLevel(LogLevel level)
{
    g_log_level = level;
}

void
SetLogFileName(const char *file_name)
{
    g_log_file_name = file_name;
    g_log_stream.open(file_name);
}

static std::atomic<uint32_t> g_current_id(0);
static std::atomic<uint32_t> g_request_id(0);

struct Waiter {
    Waiter(uint32_t id, std::atomic<uint32_t> &current_id) : id_(id), current_id_(current_id) {}

    ~Waiter() { current_id_.fetch_add(1); }

    uint32_t id() { return id_; }

    void wait()
    {
        while (id_ > current_id_.load()) {}
    }

private:
    uint32_t id_;
    std::atomic<uint32_t> &current_id_;
};

void
Log(LogLevel level, const char *tag, const char *fmt, const char *file_name, int line, const char *func_name, ...)
{
    if (level < g_log_level) { return; }

    auto utc_time  = GetCurrentUTCTime();
    auto level_str = ToShortString(level);
    int len        = file_name ? strlen(file_name) : 0;
    while (len > 0 && file_name[len - 1] != '/') { len--; }

    auto msg = fmt::format("{} {} {} {}:{}@{}: {}", utc_time, level_str, tag, file_name + len, line, func_name, fmt);

    Waiter waiter(g_request_id.fetch_add(1), g_current_id);
    waiter.wait();
    if (g_log_stream.is_open()) { g_log_stream << msg << std::endl; }
    std::cout << GetConsoleColorPrefix(level) << msg << GetConsoleColorSuffix() << std::endl;
}

}// namespace sled
