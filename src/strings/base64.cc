#include "sled/strings/base64.h"
#include "sled/synchronization/call_once.h"
#include <fmt/format.h>
#include <sstream>

namespace sled {
const char kBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static int kInvBase64Chars[(1 << sizeof(char))];
static OnceFlag once_flag;

inline bool
IsBase64(char c)
{
    return isalnum(c) || c == '+' || c == '/';
}

std::string
Base64::Encode(void *ptr, size_t len)
{
    auto data = reinterpret_cast<unsigned char *>(ptr);
    return Encode(std::vector<unsigned char>(data, data + len));
}

std::string
Base64::Encode(const std::string &input)
{
    return Encode((void *) input.data(), input.length());
}

std::string
Base64::Encode(const std::vector<unsigned char> &data)
{
    std::stringstream ss;
    int value = 0;
    int value_bits = 0;
    for (unsigned char c : data) {
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
    CallOnce(once_flag, [&] {
        std::fill(kInvBase64Chars, kInvBase64Chars + sizeof(kInvBase64Chars), -1);
        for (int i = 0; i < sizeof(kBase64Chars); i++) { kInvBase64Chars[kBase64Chars[i]] = i; }
    });

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
