#include "sled/strings/base64.h"
#include "sled/buffer.h"
#include "sled/log/log.h"
#include "sled/synchronization/call_once.h"
#include <array>
#include <fmt/format.h>
#include <sstream>
#include <string.h>
extern "C" {
#include "sled/strings/base64_decode.inc"
#include "sled/strings/base64_encode.inc"
}

namespace sled {
static constexpr int kBufferSize = 4096;
const char kBase64Chars[]        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
std::array<int, 1 << (8 * sizeof(char))> kInvBase64Chars;
// static int kInvBase64Chars[(1 << sizeof(char))];
static OnceFlag once_flag;

inline bool
IsBase64(char c)
{
    return isalnum(c) || c == '+' || c == '/';
}

size_t
Base64::DecodedLength(const char *base64_data, size_t base64_len)
{
    if (base64_len <= 0) { return 0; }
    /**
     * The number of padding characters at the end of the base64 data
     * is the number of '=' characters at the end of the base64 data.
     **/
    if (base64_data) {
        size_t padding = (4 - (base64_len % 4)) % 4;
        while (base64_data[--base64_len] == '=') { ++padding; }
        return ((base64_len + 3) / 4) * 3 - ((5 + padding * 6) / 8);
    } else {
        return base64_len / 4 * 3 + ((base64_len % 4) * 6 + 5) / 8;
    }
}

std::string
Base64::Encode(const uint8_t *ptr, size_t len)

{
    if (len == 0) { return std::string(); }
    std::string result((len + 2) / 3 * 4, '0');
    size_t write_idx = 0;

    base64_encodestate state;
    base64_init_encodestate(&state);

    // char code[kBufferSize];
    int plainlength = 0;
    int codelength  = 0;
    char *code      = const_cast<char *>(result.data()) + write_idx;
    do {
        plainlength = std::min(kBufferSize, static_cast<int>(len));
        codelength  = base64_encode_block(reinterpret_cast<const char *>(ptr), plainlength, code, &state);
        code += codelength;
        len -= plainlength;
        ptr += plainlength;
    } while (len > 0 && plainlength > 0);

    codelength = base64_encode_blockend(code, &state);

    return result;

    /*
    // std::stringstream ss;
    auto encoded_length = EncodedLength(len);
    std::string result(encoded_length, 0);
    int write_idx = 0;

    int value      = 0;
    int value_bits = 0;
    while (len > 0) {
        value = (value << 8) + *ptr;
        value_bits += 8;
        while (value_bits >= 6) {
            value_bits -= 6;
            // ss << kBase64Chars[(value >> value_bits) & 0x3F];
            result[write_idx++] = kBase64Chars[(value >> value_bits) & 0x3F];
        }
        ++ptr;
        --len;
    }

    // value_bits
    // 2           ->  4  -> (8 - value_bits - 2)
    // 4           ->  2  -> (8 - value_bits - 2)
    //
    if (value_bits > 0) {
        result[write_idx++] = kBase64Chars[(value << (6 - value_bits)) & 0x3F];
        // ss << kBase64Chars[((value << 8) >> (value_bits + 2)) & 0x3F];
    }
    // while (ss.str().size() % 4) { ss << '='; }
    while (write_idx < encoded_length) { result[write_idx++] = '='; }

    // return ss.str();
    return result;
    */
}

StatusOr<std::string>
Base64::Decode(const uint8_t *ptr, size_t len)
{
    if (len == 0) { return std::string(); }

    // FIXME: 修复快速解码在arm平台错误问题
    /*
    base64_decodestate state;
    base64_init_decodestate(&state);

    sled::BufferT<char> buffer;

    char plaintext[kBufferSize];
    int codelength  = 0;
    int plainlength = 0;
    do {
        codelength = std::min(kBufferSize, static_cast<int>(len));
        // 如果长度不足4，需要填充=
        if (codelength < 4) {
            sled::BufferT<char> temp(reinterpret_cast<const char *>(ptr), codelength);
            while (temp.size() % 4) { temp.AppendData('='); }
            plainlength = base64_decode_block(temp.data(), temp.size(), plaintext, &state);
        } else {
            // 如果长度不足4，则将这部分留到下一次处理
            codelength -= codelength % 4;
            plainlength = base64_decode_block(reinterpret_cast<const char *>(ptr), codelength, plaintext, &state);
        }
        buffer.AppendData(static_cast<const char *>(plaintext), plainlength);

        ptr += codelength;
        len -= codelength;
    } while (len > 0 && codelength > 0);

    return std::string(buffer.data(), buffer.data() + buffer.size());
    */

    CallOnce(once_flag, [&] {
        std::fill(kInvBase64Chars.begin(), kInvBase64Chars.end(), -1);
        for (int i = 0; kBase64Chars[i]; i++) { kInvBase64Chars[kBase64Chars[i]] = i; }
    });

    int write_idx = 0;
    std::string data(DecodedLength((char *) ptr, len), 0);
    // std::stringstream ss;
    int value      = 0;
    int value_bits = 0;
    int index      = 0;
    for (int i = 0; i < len; i++) {
        char c = ptr[i];
        if (-1 != kInvBase64Chars[c]) {
            // valid base64 character
            value = (value << 6) | kInvBase64Chars[c];
            value_bits += 6;
            if (value_bits >= 8) {
                data[write_idx++] = (value >> (value_bits - 8)) & 0xFF;
                // ss << char((value >> (value_bits - 8)) & 0xFF);
                value_bits -= 8;
            }
        } else if (c == '=') {
            // padding character
            break;
        } else {
            // invalid character
            return MakeStatusOr<std::string>(StatusCode::kInvalidArgument,
                                             fmt::format("Invalid character [{}] at index [{}]", (char) c, index));
        }
        ++index;
    }
    while (write_idx < data.size()) data.pop_back();

    return MakeStatusOr<std::string>(data);
}

}// namespace sled
