/**
 * @file     : one_time_event
 * @created  : Saturday Feb 03, 2024 16:51:40 CST
 * @license  : MIT
 **/

#ifndef ONE_TIME_EVENT_H
#define ONE_TIME_EVENT_H

#include "sled/synchronization/mutex.h"

namespace sled {

class OneTimeEvent {
public:
    OneTimeEvent() = default;

    bool operator()()
    {
        MutexLock lock(&mutex_);
        if (happended_) { return false; }
        happended_ = true;
        return true;
    }

private:
    bool happended_ = false;
    Mutex mutex_;
};

class ThreadUnsafeOneTimeEvent {
public:
    ThreadUnsafeOneTimeEvent() = default;

    bool operator()()
    {
        if (happended_) { return false; }
        happended_ = true;
        return true;
    }

private:
    bool happended_ = false;
};
}// namespace sled

#endif// ONE_TIME_EVENT_H
