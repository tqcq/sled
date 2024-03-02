#ifndef SLED_STRINGS_UTILS_H
#define SLED_STRINGS_UTILS_H
#include <string>

namespace sled {

std::string ToLower(const std::string &str);
std::string ToUpper(const std::string &str);
std::string ToHex(const std::string &str);
std::string StrJoin(const std::vector<std::string> &strings,
                    const std::string &delim,
                    bool skip_empty = false);

}// namespace sled
#endif//   SLED_STRINGS_UTILS_H
