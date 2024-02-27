#include "sled/time_utils.h"
#include "sled/system_time.h"
#include <sys/time.h>
#include <time.h>

namespace sled {

ClockInterface *g_clock = nullptr;

int64_t
TimeSecs()
{
    return TimeNanos() / kNumNanosecsPerSec;
}

int64_t
TimeMillis()
{
    return TimeNanos() / kNumNanosecsPerMillisec;
}

int64_t
TimeMicros()
{
    return TimeNanos() / kNumNanosecsPerMicrosec;
}

int64_t
TimeNanos()
{
    if (g_clock) { return g_clock->TimeNanos(); }
    return SystemTimeNanos();
}

int64_t
TimeAfterMillis(int64_t elapsed)
{
    return TimeMillis() + elapsed;
}

int64_t
TimeDiff(int64_t later, int64_t earlier)
{
    return later - earlier;
}

int32_t
TimeDiff(int32_t later, int32_t earlier)
{
    return later - earlier;
}

int64_t
TmToSeconds(const tm &tm_val)
{
    struct tm *ptm = const_cast<struct tm *>(&tm_val);
    return ::timegm(ptm);
}

int64_t
TimeUTCSeconds()
{
    return TimeUTCNanos() / kNumNanosecsPerSec;
}

int64_t
TimeUTCMicros()
{
    return TimeUTCNanos() / kNumNanosecsPerMicrosec;
}

int64_t
TimeUTCMillis()
{
    return TimeUTCNanos() / kNumNanosecsPerMillisec;
}

int64_t
TimeUTCNanos()
{
    if (g_clock) { return g_clock->TimeNanos() / kNumNanosecsPerMicrosec; }
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_nsec + ts.tv_sec * kNumNanosecsPerSec;
}

}// namespace sled
