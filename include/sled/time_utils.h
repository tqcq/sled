/**
 * @file     : time_units
 * @created  : Saturday Feb 03, 2024 15:20:05 CST
 * @license  : MIT
 **/

#ifndef TIME_UNITS_H
#define TIME_UNITS_H
#include <stdint.h>
#include <time.h>

namespace sled {
static const int64_t kNumMillisecsPerSec = 1000;
static const int64_t kNumMicrosecsPerSec = 1000000;
static const int64_t kNumNanosecsPerSec = 1000000000;
static const int64_t kNumMicrosecsPerMillisec =
    kNumMicrosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMillisec =
    kNumNanosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMicrosec =
    kNumNanosecsPerSec / kNumMicrosecsPerSec;
constexpr int64_t kNtpJan1970Millisecs = 2208988800 * kNumMillisecsPerSec;

class ClockInterface {
public:
    virtual ~ClockInterface() = default;
    virtual int64_t TimeNanos() const = 0;
};

int64_t SystemTimeMillis();
int64_t TimeSecs();
int64_t TimeMillis();
int64_t TimeMicros();
int64_t TimeNanos();
int64_t TimeAfterMillis(int64_t elapsed);
int64_t TimeDiff(int64_t later, int64_t earlier);
int32_t TimeDiff(int32_t later, int32_t earlier);

inline int64_t
TimeSinceMillis(int64_t earlier)
{
    return TimeMillis() - earlier;
}

inline int64_t
TimeSinceMicros(int64_t earlier)
{
    return TimeMicros() - earlier;
}

inline int64_t
TimeSinceNanos(int64_t earlier)
{
    return TimeNanos() - earlier;
}

inline int64_t
TimeUntilMillis(int64_t later)
{
    return later - TimeMillis();
}

inline int64_t
TimeUntilMicros(int64_t later)
{
    return later - TimeMicros();
}

inline int64_t
TimeUntilNanos(int64_t later)
{
    return later - TimeNanos();
}

int64_t TmToSeconds(const tm &tm);
int64_t TimeUTCSeconds();
int64_t TimeUTCMicros();
int64_t TimeUTCMillis();
int64_t TimeUTCNanos();

}// namespace sled

#endif// TIME_UNITS_H
