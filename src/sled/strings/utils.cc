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
ToLower(sled::string_view str)
{
    std::stringstream ss;
    for (auto &ch : str) { ss << ToLower(ch); }
    return ss.str();
}

std::string
ToUpper(sled::string_view str)
{
    std::stringstream ss;
    for (auto &ch : str) { ss << ToUpper(ch); }
    return ss.str();
}

std::string
StrJoin(const std::vector<std::string> &strings, const std::string &delim, bool skip_empty)
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

    size_t start    = 0;
    size_t next_pos = str.find_first_of(delim, start);
    while (next_pos != std::string::npos) {
        if ((!skip_empty && next_pos == start) || next_pos > start) {
            result.emplace_back(str.begin() + start, str.begin() + next_pos);
        }

        if (!skip_empty) {
            start    = next_pos + 1;
            next_pos = str.find_first_of(delim, start);
        } else {
            start = str.find_first_not_of(delim, next_pos);
            if (start == std::string::npos) {
                // all remaining characters are delimiters
                break;
            }
            next_pos = str.find_first_of(delim, start);
        }
    }

    if (start < str.size()) {
        result.emplace_back(str.substr(start));
    } else if (!skip_empty && !str.empty() && delim.find(str.back()) != std::string::npos) {
        result.emplace_back("");
    }
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

// bool
// EndsWith(const std::string &str, const std::string &suffix)
// {
//     return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
// }
//
// bool
// StartsWith(const std::string &str, const std::string &prefix)
// {
//     return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
// }
bool
StartsWith(sled::string_view str, sled::string_view prefix)
{
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

bool
EndsWith(sled::string_view str, sled::string_view suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// bool
// EndsWithIgnoreCase(const std::string &str, const std::string &suffix)
// {
//     return EndsWith(ToLower(str), ToLower(suffix));
// }
//
// bool
// StartsWithIgnoreCase(const std::string &str, const std::string &prefix)
// {
//     return StartsWith(ToLower(str), ToLower(prefix));
// }

bool
EndsWithIgnoreCase(sled::string_view str, sled::string_view suffix)
{
    if (str.size() < suffix.size()) { return false; }

    auto str_iter    = str.rbegin();
    auto suffix_iter = suffix.rbegin();

    for (; suffix_iter != suffix.rend(); ++str_iter, ++suffix_iter) {
        if (ToLower(*str_iter) != ToLower(*suffix_iter)) { return false; }
    }
    return true;
}

bool
StartsWithIgnoreCase(sled::string_view str, sled::string_view prefix)
{
    if (str.size() < prefix.size()) { return false; }
    auto str_iter    = str.begin();
    auto prefix_iter = prefix.begin();
    for (; prefix_iter != prefix.end(); ++str_iter, ++prefix_iter) {
        if (ToLower(*str_iter) != ToLower(*prefix_iter)) { return false; }
    }
    return true;
}

bool
EqualsIgnoreCase(sled::string_view lhs, sled::string_view rhs)
{
    if (lhs.size() != rhs.size()) { return false; }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (ToLower(lhs[i]) != ToLower(rhs[i])) { return false; }
    }
    return true;
}

bool
EqualsIgnoreCase(const std::string &lhs, const std::string &rhs)
{
    return ToLower(lhs) == ToLower(rhs);
}

}// namespace sled
