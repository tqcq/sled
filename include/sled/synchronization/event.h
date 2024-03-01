/**
 * @file     : event
 * @created  : Wednesday Feb 14, 2024 13:38:08 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_SYNCHRONIZATION_EVENT_H
#define SLED_SYNCHRONIZATION_EVENT_H

#include "sled/synchronization/mutex.h"
#include "sled/units/time_delta.h"

namespace sled {

class Event {
public:
    static constexpr TimeDelta kForever = ConditionVariable::kForever;
    Event();
    Event(bool manual_reset, bool initially_signaled);
    Event(const Event &) = delete;
    Event &operator=(const Event &) = delete;
    ~Event();

    void Set();
    void Reset();
    bool Wait(TimeDelta give_up_after, TimeDelta warn_after);

    bool Wait(TimeDelta give_up_after)
    {
        return Wait(give_up_after,
                    give_up_after.IsPlusInfinity() ? TimeDelta::Seconds(3)
                                                   : kForever);
    }

    Mutex mutex_;
    ConditionVariable cv_;
    const bool is_manual_reset_;
    bool event_status_;
};

}// namespace sled

#endif// SLED_SYNCHRONIZATION_EVENT_H
