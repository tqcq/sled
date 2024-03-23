/**
 * @file     : base64
 * @created  : Monday Feb 19, 2024 15:32:59 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_STRINGS_BASE64_H
#define SLED_STRINGS_BASE64_H

#include "sled/status_or.h"
#include <string.h>
#include <string>
#include <vector>

namespace sled {

class Base64 {
public:
    static size_t DecodedLength(const char *base64_data, size_t base64_len);
    static std::string Encode(const uint8_t *ptr, size_t len);
    static StatusOr<std::string> Decode(const uint8_t *ptr, size_t len);

    // EncodedLength
    static inline size_t EncodedLength(size_t data_len) { return (data_len + 2) / 3 * 4; }

    static inline size_t DecodedLength(const std::string &str) { return DecodedLength(str.data(), str.size()); }

    static inline size_t DecodedLength(const char *base64_str) { return DecodedLength(base64_str, strlen(base64_str)); }

    // Encode
    static inline std::string Encode(const std::vector<unsigned char> &data)
    {
        return Encode(data.data(), data.size());
    }

    static inline std::string Encode(const std::string &data) { return Encode((uint8_t *) data.data(), data.size()); }

    static inline std::string Encode(const char *const data) { return Encode((uint8_t *) data, strlen(data)); }

    // Decode
    static inline StatusOr<std::string> Decode(const char *ptr, size_t len)
    {
        return Decode((const uint8_t *) ptr, len);
    }

    static inline StatusOr<std::string> Decode(const char *ptr) { return Decode(ptr, strlen(ptr)); }

    static inline StatusOr<std::string> Decode(const std::string &base64)
    {
        return Decode(base64.data(), base64.size());
    }

    static inline StatusOr<std::string> Decode(const std::vector<unsigned char> &base64)
    {
        return Decode(base64.data(), base64.size());
    }
};

}// namespace sled

#endif// SLED_STRINGS_BASE64_H
