#include "sled/system/location.h"

namespace sled {

Location
Location::Current(const char *file_name, int file_line, const char *function)
{
    return Location(file_name, file_line, function);
}

Location::Location(const char *file_name, int file_line, const char *function)
    : file_name(file_name),
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
