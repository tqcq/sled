#include "sled/strings/base64.h"
#include <fmt/format.h>
#include <sstream>

namespace sled {
const char kBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const int kInvBase64Chars[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
    -1, -1, -1, -1, -1, -1, -1, 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
    22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
    45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

inline bool
IsBase64(char c)
{
    return isalnum(c) || c == '+' || c == '/';
}

std::string
Base64::Encode(const std::string &input)
{
    std::stringstream ss;
    const unsigned char *data = reinterpret_cast<const unsigned char *>(input.c_str());
    int value = 0;
    int value_bits = 0;
    for (unsigned char c : input) {
        value = (value << 8) + c;
        value_bits += 8;
        while (value_bits >= 6) {
            value_bits -= 6;
            ss << kBase64Chars[(value >> value_bits) & 0x3F];
        }
    }

    /**
     * value_bits      
     * 2           ->  4  -> (8 - value_bits - 2)
     * 4           ->  2  -> (8 - value_bits - 2)
     **/
    if (value_bits > 0) { ss << kBase64Chars[((value << 8) >> (value_bits + 2)) & 0x3F]; }
    while (ss.str().size() % 4) { ss << '='; }

    return ss.str();
}

StatusOr<std::string>
Base64::Decode(const std::string &input)
{
    std::stringstream ss;
    int value = 0;
    int value_bits = 0;
    int index = 0;
    for (unsigned char c : input) {
        if (-1 != kInvBase64Chars[c]) {
            // valid base64 character
            value = (value << 6) | kInvBase64Chars[c];
            value_bits += 6;
            if (value_bits >= 8) {
                ss << char((value >> (value_bits - 8)) & 0xFF);
                value_bits -= 8;
            }
        } else if (c == '=') {
            // padding character
            break;
        } else {
            // invalid character
            return make_status_or<std::string>(StatusCode::kInvalidArgument,
                                               fmt::format("Invalid character [{}] at index [{}]", (char) c, index));
        }
        ++index;
    }

    return make_status_or<std::string>(ss.str());
}
}// namespace sled
