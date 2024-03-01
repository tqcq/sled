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

namespace sled {

class Base64 {
public:
    static std::string Encode(const std::string &input);
    static StatusOr<std::string> Decode(const std::string &input);
};

}// namespace sled

#endif// SLED_STRINGS_BASE64_H
