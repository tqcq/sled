#pragma once
#ifndef SLED_TIMER_TIMEOUT_H
#define SLED_TIMER_TIMEOUT_H

#include <stdint.h>

namespace sled {
typedef uint32_t DurationMs;
typedef uint64_t TimeoutID;

class Timeout {
public:
    virtual ~Timeout() = default;
    virtual void Start(DurationMs duration, TimeoutID timeout_id) = 0;
    virtual void Stop() = 0;

    virtual void Restart(DurationMs duration, TimeoutID timeout_id)
    {
        Stop();
        Start(duration, timeout_id);
    }
};
}// namespace sled
 //
#endif// SLED_TIMER_TIMEOUT_H
