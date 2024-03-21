#pragma once
#include "sled/scoped_refptr.h"
#ifndef SLED_TIMER_QUEUE_TIMEOUT_H
#define SLED_TIMER_QUEUE_TIMEOUT_H

#include "sled/task_queue/pending_task_safety_flag.h"
#include "sled/task_queue/task_queue_base.h"
#include "sled/timer/timeout.h"
#include <limits>
#include <memory>

namespace sled {
typedef uint64_t TimeMs;

class TaskQueueTimeoutFactory {
public:
    TaskQueueTimeoutFactory(sled::TaskQueueBase &task_queue,
                            std::function<TimeMs()> get_time,
                            std::function<void(TimeoutID timeout_id)> on_expired)
        : task_queue_(task_queue),
          get_time_(get_time),
          on_expired_(on_expired)
    {}

    std::unique_ptr<Timeout>
    CreateTimeout(sled::TaskQueueBase::DelayPrecision precision = sled::TaskQueueBase::DelayPrecision::kHigh)
    {
        return std::unique_ptr<TaskQueueTimeout>(new TaskQueueTimeout(*this, precision));
    }

private:
    class TaskQueueTimeout : public Timeout {
    public:
        TaskQueueTimeout(TaskQueueTimeoutFactory &parent, sled::TaskQueueBase::DelayPrecision precision);
        ~TaskQueueTimeout() override;
        void Start(DurationMs duration, TimeoutID timeout_id) override;
        void Stop() override;

    private:
        TaskQueueTimeoutFactory &parent_;
        const sled::TaskQueueBase::DelayPrecision precision_;
        TimeMs posted_task_expiration_ = std::numeric_limits<TimeMs>::max();
        TimeMs timeout_expiration_ = std::numeric_limits<TimeMs>::max();
        TimeoutID timeout_id_ = TimeoutID(0);
        scoped_refptr<PendingTaskSafetyFlag> safety_flag_;
    };

    sled::TaskQueueBase &task_queue_;
    const std::function<TimeMs()> get_time_;
    const std::function<void(TimeoutID)> on_expired_;
};
}// namespace sled
#endif//  SLED_TIMER_QUEUE_TIMEOUT_H
