#include "sled/uri.h"
#include "sled/strings/utils.h"

namespace sled {
URI
URI::ParseURI(const std::string &uri_str)
{
    if (uri_str.empty()) { return {}; }
    URI uri;
    // TODO: decode before
    auto start_pos = uri_str.find_first_not_of(" ");
    auto end_pos = uri_str.find(':');

    return std::move(uri);
}
}// namespace sled
