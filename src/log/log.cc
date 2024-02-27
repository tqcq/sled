#include "sled/log/log.h"
#include "sled/time_utils.h"
#include <ctime>
#include <fmt/format.h>
#include <iostream>

namespace sled {

class ScopedAtomicWaiter {
public:
    ScopedAtomicWaiter(std::atomic_bool &flag) : flag_(flag)
    {
        bool old = flag_.load();
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
    std::time_t t = now / kNumNanosecsPerSec;
#else
    std::time_t t = std::time(nullptr);
#endif
    std::tm tm = *std::gmtime(&t);
    char buf[64];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S", &tm);
    std::string result(buf);
#if true
    return result
        + fmt::format(".{:03d}Z",
                      static_cast<int>(now % kNumNanosecsPerSec)
                          / kNumNanosecsPerMillisec);
#else
    return result + "Z";
#endif
    return result;
}

void
Log(LogLevel level,
    const char *tag,
    const char *fmt,
    const char *file_name,
    int line,
    const char *func_name,
    ...)
{
    static std::atomic_bool allow(true);
    ScopedAtomicWaiter waiter(allow);
    int len = file_name ? strlen(file_name) : 0;
    while (len > 0 && file_name[len - 1] != '/') { len--; }

    auto msg = fmt::format("{} {}:{}@{} {} {}", GetCurrentUTCTime(),
                           file_name + len, line, func_name, tag, fmt);
    std::cout << msg << std::endl;
}

}// namespace sled
