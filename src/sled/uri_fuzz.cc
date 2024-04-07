#include <sled/sled.h>
#include <string>

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size > 15) { return 1; }
    auto uri_or = sled::URI::ParseURI(std::string(reinterpret_cast<const char *>(data), size));
    if (!uri_or.ok()) { return 0; }
    auto uri = std::move(uri_or.value());
    // sled::URI uri(std::string(reinterpret_cast<const char *>(data), size));
    int cnt = 0;
    if (!uri.scheme().empty()) { cnt += 1 << 12; }
    if (!uri.username().empty()) { cnt += 1 << 11; }
    if (!uri.password().empty()) { cnt += 1 << 10; }
    if (!uri.host().empty()) { cnt += 1 << 9; }
    if (uri.port() != 0) { cnt += 1 << 8; }
    if (!uri.path().empty()) { cnt += 1 << 7; }
    if (!uri.query().empty()) { cnt += uri.query().size(); }
    if (!uri.anchor().empty()) { cnt += 1 << 5; }
    return 0;
}
