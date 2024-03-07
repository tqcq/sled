#include "sled/system/location.h"
#include <string.h>

namespace sled {
size_t
StrippedPathLength()
{
#ifdef __SLED_LOCATION_STRIPPED_PATH
    const char *my_path = __SLED_LOCATION_STRIPPED_PATH;
#else
    const char *my_path = "sled/src/system/location.cc";
#endif
    static size_t suffix_len = strlen(my_path);
    static size_t full_len = strlen(__FILE__);
    return full_len - suffix_len;
}

Location
Location::Current(const char *file_name, int file_line, const char *function)
{
    return Location(file_name, file_line, function);
}

Location::Location(const char *file_name, int file_line, const char *function)
    : file_name(file_name + StrippedPathLength()),
      file_line(file_line),
      function(function)
{}

std::string
Location::ToString() const
{
    return std::string(file_name) + ":" + std::to_string(file_line) + " "
        + function;
}
}// namespace sled
