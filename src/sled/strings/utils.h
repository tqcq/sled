#pragma once
#ifndef SLED_STRINGS_UTILS_H
#define SLED_STRINGS_UTILS_H
#include "sled/nonstd/string_view.h"
#include <string>
#include <vector>

namespace sled {

char ToLower(char c);
char ToUpper(char c);
// std::string ToLower(const std::string &str);
// std::string ToUpper(const std::string &str);
std::string ToLower(sled::string_view str);
std::string ToUpper(sled::string_view str);
std::string ToHex(const std::string &str);
std::string StrJoin(const std::vector<std::string> &strings, const std::string &delim, bool skip_empty = false);
std::vector<std::string> StrSplit(const std::string &str, const std::string &delim, bool skip_empty = false);
std::string Trim(const std::string &str, const std::string &chars = " \t\n\r");
std::string TrimLeft(const std::string &str, const std::string &chars = " \t\n\r");
std::string TrimRight(const std::string &str, const std::string &chars = " \t\n\r");
// bool StartsWith(const std::string &str, const std::string &prefix);
// bool EndsWith(const std::string &str, const std::string &suffix);
bool StartsWith(sled::string_view str, sled::string_view prefix);
bool EndsWith(sled::string_view str, sled::string_view suffix);

// bool EndsWithIgnoreCase(const std::string &str, const std::string &suffix);
// bool StartsWithIgnoreCase(const std::string &str, const std::string &prefix);
bool EndsWithIgnoreCase(sled::string_view str, sled::string_view suffix);
bool StartsWithIgnoreCase(sled::string_view str, sled::string_view prefix);

// bool EqualsIgnoreCase(const std::string &lhs, const std::string &rhs);
bool EqualsIgnoreCase(sled::string_view lhs, sled::string_view rhs);

inline sled::string_view
StripPrefix(sled::string_view str, sled::string_view prefix)
{
    if (sled::StartsWith(str, prefix)) str.remove_prefix(prefix.size());
    return str;
}

inline sled::string_view
StripSuffix(sled::string_view str, sled::string_view suffix)
{
    if (sled::EndsWith(str, suffix)) str.remove_suffix(suffix.size());
    return str;
}
}// namespace sled
#endif//   SLED_STRINGS_UTILS_H
