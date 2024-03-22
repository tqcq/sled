#include <sled/log/log.h>
#include <sled/strings/base64.h>
#include <stdlib.h>
#include <string.h>

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    std::string encoded = sled::Base64::Encode((void *) data, size);

    auto decoded_or = sled::Base64::Decode(encoded);

    ASSERT(decoded_or.ok(), "decoded failed");
    auto decoded = decoded_or.value();
    ASSERT(decoded.size() == size, "decoded size mismatch");
    ASSERT(memcmp(decoded.data(), data, size) == 0, "decoded data mismatch");
    return 0;
}
