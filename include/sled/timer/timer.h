#pragma once
#ifndef SLED_TIMER_TIMER_H
#define SLED_TIMER_TIMER_H

#include "timeout.h"
#include <map>
#include <memory>
#include <sled/optional.h>
#include <sled/task_queue/task_queue_base.h>
#include <stdint.h>

namespace sled {
typedef uint64_t TimerID;
typedef uint32_t TimerGeneration;

class Timer {
public:
    using OnExpired = std::function<sled::optional<DurationMs>()>;
    Timer(const Timer &) = delete;
    Timer &operator=(const Timer &) = delete;
    ~Timer();
    void Start();
    void Stop();

    void set_duration(DurationMs duration) { duration_ = duration; }

    const DurationMs &duration() const { return duration_; }

    int expireation_count() const { return expiration_count_; }

    bool is_running() const { return is_running_; }

private:
    friend class TimerManager;
    using UnregisterHandler = std::function<void()>;
    Timer(TimerID id,
          const std::string &name,
          OnExpired on_expired,
          UnregisterHandler unregister_handler,
          std::unique_ptr<Timeout> timeout);

    const TimerID id_;
    const std::string name_;
    const OnExpired on_expired_;
    const UnregisterHandler unregister_handler_;
    std::unique_ptr<Timeout> timeout_;

    DurationMs duration_;

    TimerGeneration generation_ = TimerGeneration(0);
    bool is_running_ = false;
    int expiration_count_ = 0;
};

class TimerManager {
    using TimeoutCreator = std::function<std::unique_ptr<Timeout>(
        sled::TaskQueueBase::DelayPrecision)>;

public:
    explicit TimerManager(TimeoutCreator timeout_creator)
        : timeout_creator_(timeout_creator)
    {}

    std::unique_ptr<Timer> CreateTimer(const std::string &name,
                                       Timer::OnExpired on_expired);
    void HandleTimeout(TimeoutID timeout_id);

private:
    const TimeoutCreator timeout_creator_;
    std::map<TimerID, Timer *> timers_;
    TimerID next_id_ = TimerID(0);
};
}// namespace sled
#endif//  SLED_TIMER_TIMER_H
