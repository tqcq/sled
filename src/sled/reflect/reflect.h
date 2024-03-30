/**
 * @file     : reflect
 * @created  : 星期一  2 26, 2024 09:55:19 CST
 * @license  : MIT
 **/

#ifndef SLED_REFLECT_REFLECT_H
#define SLED_REFLECT_REFLECT_H
#pragma
#include <cxxabi.h>
#include <string>

#if !defined(__NO_META_PARSER__) && !defined(__META_PARSER__)
#define __META_PARSER__
#endif

#if defined(__META_PARSER__)
#define CLASS(...) class __attribute__((annotate("reflect-class"))) #__VA_ARGS__
#define PROPERTY() __attribute__((annotate("reflect-property")))
#define METHOD() __attribute__((annotate("reflect-method")))
#else
#define REFLECT_CLASS
#define PROPERTY()
#define METHOD()
#endif

namespace sled {

namespace {

inline std::string
RemoveUnusedSpace(const std::string &str, const std::string &chars = "[]()<>*&:")
{
    std::string result = str.substr(str.find_first_not_of(' '));
    result = result.substr(0, result.find_last_not_of(' ') + 1);
    for (size_t i = 0; i < result.size();) {
        if (result[i] == ' ') {
            bool left_is_chars = i > 0 && chars.find(result[i - 1]) != std::string::npos;
            bool right_is_chars = (i + 1 < str.size()) && chars.find(result[i + 1]) != std::string::npos;
            if (left_is_chars || right_is_chars) {
                result.erase(i, 1);
                continue;
            }
        }
        ++i;
    }
    return result;
}

template<typename T, typename U = typename std::decay<T>::type>
inline std::string
PrettyNameImpl()
{
    const char *name = typeid(U).name();
    int status;
    char *res = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string pretty_name = res;
    free(res);
    return RemoveUnusedSpace(pretty_name);
}
}// namespace

template<typename T>
inline const char *
PrettyName()
{
    static std::string pretty_name = PrettyNameImpl<T>();
    return pretty_name.c_str();
}
}// namespace sled

#endif// SLED_REFLECT_REFLECT_H
