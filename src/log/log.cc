#include "sled/log/log.h"
#include <fmt/format.h>
#include <iostream>

namespace sled {
void
Log(LogLevel level,
    const char *tag,
    const char *fmt,
    const char *file_name,
    int line,
    const char *func_name,
    ...)
{
    int len = file_name ? strlen(file_name) : 0;
    while (len > 0 && file_name[len - 1] != '/') { len--; }

    auto msg = fmt::format("{}:{}@{} {} {}", file_name + len, line, func_name,
                           tag, fmt);
    std::cout << msg << std::endl;
}

}// namespace sled
