#include "sled/strings/utils.h"
#include <sstream>

namespace sled {

std::string
StrJoin(const std::vector<std::string> &strings,
        const std::string &delim,
        bool skip_empty)
{
    if (strings.empty()) { return ""; }

    std::stringstream ss;
    size_t i = 0;
    while (skip_empty && i < strings.size() && strings[i].empty()) { ++i; }
    if (i < strings.size()) { ss << strings[i++]; }
    for (; i < strings.size(); ++i) {
        if (skip_empty && strings[i].empty()) { continue; }
        ss << delim << strings[i];
    }
    return ss.str();
}

std::vector<std::string>
StrSplit(const std::string &str, const std::string &delim, bool skip_empty)
{
    std::vector<std::string> result;
    if (str.empty()) { return result; }

    size_t start = 0;
    size_t next_pos = str.find_first_of(delim, start);
    while (next_pos != std::string::npos) {
        if (!skip_empty || next_pos > start) {
            result.emplace_back(str.substr(start, next_pos - start));
        }
        start = next_pos + 1;
        next_pos = str.find_first_of(delim, start);
    }

    if (start < str.size()) { result.emplace_back(str.substr(start)); }
    return result;
}

std::string
Trim(const std::string &str, const std::string &chars)
{
    return TrimLeft(TrimRight(str));
}

std::string
TrimLeft(const std::string &str, const std::string &chars)
{
    size_t start = 0;
    while (start < str.size() && chars.find(str[start]) != std::string::npos) {
        ++start;
    }
    return str.substr(start);
}

std::string
TrimRight(const std::string &str, const std::string &chars)
{
    size_t end = str.size();
    while (end > 0 && chars.find(str[end - 1]) != std::string::npos) { --end; }
    return str.substr(0, end);
}

}// namespace sled
