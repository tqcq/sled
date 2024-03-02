#include "sled/strings/utils.h"
#include <sstream>

namespace sled {

std::string
StrJoin(const std::vector<std::string> &strings,
        const std::string &delim,
        bool skip_empty)
{
    if (strings.empty()) { return ""; }

    std::stringstream ss;
    size_t i = 0;
    while (skip_empty && i < strings.size() && strings[i].empty()) { ++i; }
    if (i < strings.size()) { ss << strings[i++]; }
    for (; i < strings.size(); ++i) {
        if (skip_empty && strings[i].empty()) { continue; }
        ss << delim << strings[i];
    }
    return ss.str();
}
}// namespace sled
