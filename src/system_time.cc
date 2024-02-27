#include "sled/system_time.h"
#include "sled/time_utils.h"

namespace sled {
int64_t
SystemTimeNanos()
{
    int64_t ticks;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ticks = kNumNanosecsPerSec * static_cast<int64_t>(ts.tv_sec)
        + static_cast<int64_t>(ts.tv_nsec);
    return ticks;
}
}// namespace sled
