#include "sled/units/timestamp.h"
#include <sstream>

namespace sled {
std::string
ToString(Timestamp value)
{
    char buf[64];
    std::stringstream ss;
    if (value.IsPlusInfinity()) {
        ss << "+inf ms";
    } else if (value.IsMinusInfinity()) {
        ss << "-inf ms";
    } else {
        if (value.us() == 0 || (value.us() % 1000) != 0)
            ss << value.us() << " us";
        else if (value.ms() % 1000 != 0)
            ss << value.ms() << " ms";
        else
            ss << value.seconds() << " s";
    }
    return ss.str();
}
}// namespace sled
