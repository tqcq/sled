#pragma once
#ifndef SLED_STRINGS_UTILS_H
#define SLED_STRINGS_UTILS_H
#include <string>
#include <vector>

namespace sled {

char ToLower(char c);
char ToUpper(char c);
std::string ToLower(const std::string &str);
std::string ToUpper(const std::string &str);
std::string ToHex(const std::string &str);
std::string StrJoin(const std::vector<std::string> &strings, const std::string &delim, bool skip_empty = false);
std::vector<std::string> StrSplit(const std::string &str, const std::string &delim, bool skip_empty = false);
std::string Trim(const std::string &str, const std::string &chars = " \t\n\r");
std::string TrimLeft(const std::string &str, const std::string &chars = " \t\n\r");
std::string TrimRight(const std::string &str, const std::string &chars = " \t\n\r");
bool EndsWith(const std::string &str, const std::string &suffix);
bool StartsWith(const std::string &str, const std::string &prefix);

bool EndsWithIgnoreCase(const std::string &str, const std::string &suffix);
bool StartsWithIgnoreCase(const std::string &str, const std::string &prefix);

bool EqualsIgnoreCase(const std::string &lhs, const std::string &rhs);

}// namespace sled
#endif//   SLED_STRINGS_UTILS_H
