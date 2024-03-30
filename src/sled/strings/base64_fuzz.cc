#include <sled/log/log.h>
#include <sled/strings/base64.h>
#include <stdlib.h>
#include <string.h>

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < 1) { return 0; }

    std::string encoded;
    sled::StatusOr<std::string> decoded_or;
    switch ((*data & 2)) {
    case 0:
        encoded = sled::Base64::Encode(data + 1, size - 1);
        break;
    case 1:
        encoded = sled::Base64::Encode(std::vector<uint8_t>(data + 1, data + size));
        break;
    case 2:
        encoded = sled::Base64::Encode(std::string((char *) data + 1, size - 1));
        break;
    }
    switch ((*data >> 2) & 2) {
    case 0:
        decoded_or = sled::Base64::Decode(encoded);
        break;
    case 1:
        decoded_or = sled::Base64::Decode((uint8_t *) encoded.data(), encoded.size());
        break;
    case 2:
        decoded_or = sled::Base64::Decode(std::vector<uint8_t>(encoded.begin(), encoded.end()));
        break;
    }

    ASSERT(decoded_or.ok(), "decoded failed");
    auto decoded = decoded_or.value();
    ASSERT(decoded.size() == size - 1, "decoded size mismatch");
    ASSERT(memcmp(decoded.data(), data + 1, size - 1) == 0, "decoded data mismatch");
    return 0;
}
