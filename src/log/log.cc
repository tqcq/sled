#include "sled/log/log.h"
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

    auto msg = fmt::format("{}:{}@{} {} {}", file_name + len, line, func_name,
                           tag, fmt);
    std::cout << msg << std::endl;
}

}// namespace sled
