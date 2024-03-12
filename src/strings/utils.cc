#include "sled/strings/utils.h"
#include <sstream>
#include <string.h>

namespace sled {

char
ToLower(char c)
{
    return ::tolower(c);
}

char
ToUpper(char c)
{
    return ::toupper(c);
}

std::string
ToLower(const std::string &str)
{
    std::stringstream ss;
    for (auto &ch : str) { ss << ToLower(ch); }
    return ss.str();
}

std::string
ToUpper(const std::string &str)
{
    std::stringstream ss;
    for (auto &ch : str) { ss << ToUpper(ch); }
    return ss.str();
}

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
    return TrimLeft(TrimRight(str, chars), chars);
}

std::string
TrimLeft(const std::string &str, const std::string &chars)
{
    size_t start = str.find_first_not_of(chars);
    return start == std::string::npos ? "" : str.substr(start);
}

std::string
TrimRight(const std::string &str, const std::string &chars)
{
    size_t end = str.find_last_not_of(chars);
    return end == std::string::npos ? "" : str.substr(0, end + 1);
}

bool
EndsWith(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size()
        && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool
StartsWith(const std::string &str, const std::string &prefix)
{
    return str.size() >= prefix.size()
        && str.compare(0, prefix.size(), prefix) == 0;
}

}// namespace sled
