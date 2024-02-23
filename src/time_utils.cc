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
    if (g_clock) { return g_clock->TimeNanos(); }
    return TimeNanos() / kNumNanosecsPerMicrosec;
}

int64_t
TimeNanos()
{
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
TmToSeconds(const tm &tm)
{
    static short int mdays[12] = {31, 28, 31, 30, 31, 30,
                                  31, 31, 30, 31, 30, 31};
    static short int cumul_mdays[12] = {0,   31,  59,  90,  120, 151,
                                        181, 212, 243, 273, 304, 334};
    int year = tm.tm_year + 1900;
    int month = tm.tm_mon;
    int day = tm.tm_mday - 1;// Make 0-based like the rest.
    int hour = tm.tm_hour;
    int min = tm.tm_min;
    int sec = tm.tm_sec;

    bool expiry_in_leap_year =
        (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));

    if (year < 1970) return -1;
    if (month < 0 || month > 11) return -1;
    if (day < 0
        || day >= mdays[month] + (expiry_in_leap_year && month == 2 - 1))
        return -1;
    if (hour < 0 || hour > 23) return -1;
    if (min < 0 || min > 59) return -1;
    if (sec < 0 || sec > 59) return -1;

    day += cumul_mdays[month];

    // Add number of leap days between 1970 and the expiration year, inclusive.
    day += ((year / 4 - 1970 / 4) - (year / 100 - 1970 / 100)
            + (year / 400 - 1970 / 400));

    // We will have added one day too much above if expiration is during a leap
    // year, and expiration is in January or February.
    if (expiry_in_leap_year && month <= 2 - 1)// `month` is zero based.
        day -= 1;

    // Combine all variables into seconds from 1970-01-01 00:00 (except `month`
    // which was accumulated into `day` above).
    return (((static_cast<int64_t>(year - 1970) * 365 + day) * 24 + hour) * 60
            + min)
        * 60
        + sec;
    return -1;
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
    struct timeval time;
    gettimeofday(&time, nullptr);
    int64_t nanosecs =
        static_cast<int64_t>(time.tv_sec) * kNumNanosecsPerSec + time.tv_usec;
    return nanosecs;
}

}// namespace sled
