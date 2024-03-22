/**
 * @file     : base64
 * @created  : Monday Feb 19, 2024 15:32:59 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_STRINGS_BASE64_H
#define SLED_STRINGS_BASE64_H

#include "sled/status_or.h"
#include <string>
#include <vector>

namespace sled {

class Base64 {
public:
    static std::string Encode(void *ptr, size_t len);
    static std::string Encode(const std::vector<unsigned char> &data);
    static std::string Encode(const std::string &data);
    static StatusOr<std::string> Decode(const std::string &base64);
};

}// namespace sled

#endif// SLED_STRINGS_BASE64_H
