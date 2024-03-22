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
    static std::string Encode(const uint8_t *const ptr, size_t len);
    static std::string Encode(const std::vector<unsigned char> &data);
    static std::string Encode(const std::string &data);
    static std::string Encode(const char *const data);
    static StatusOr<std::string> Decode(const std::string &base64);
    static StatusOr<std::string> Decode(const std::vector<unsigned char> &base64);
    static StatusOr<std::string> Decode(const uint8_t *const ptr, size_t len);
};

}// namespace sled

#endif// SLED_STRINGS_BASE64_H
