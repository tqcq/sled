#include "sled/timer/task_queue_timeout.h"
#include "sled/log/log.h"
#include "sled/units/time_delta.h"

namespace sled {
TaskQueueTimeoutFactory::TaskQueueTimeout::TaskQueueTimeout(TaskQueueTimeoutFactory &parent,
                                                            sled::TaskQueueBase::DelayPrecision precision)
    : parent_(parent),
      precision_(precision)
{}

TaskQueueTimeoutFactory::TaskQueueTimeout::~TaskQueueTimeout() {}

void
TaskQueueTimeoutFactory::TaskQueueTimeout::Start(DurationMs duration_ms, TimeoutID timeout_id)
{
    ASSERT(timeout_expiration_ == std::numeric_limits<TimeMs>::max(), "");
    timeout_expiration_ = parent_.get_time_() + duration_ms;
    timeout_id_ = timeout_id;

    if (timeout_expiration_ >= posted_task_expiration_) { return; }
    if (posted_task_expiration_ != std::numeric_limits<TimeMs>::max()) {
        LOGV("timer",
             "New timeout duration is less than scheduled - "
             "ghosting old delayed task");
    }

    posted_task_expiration_ = timeout_expiration_;
    parent_.task_queue_.PostDelayedTaskWithPrecision(
        precision_,
        [timeout_id, this]() {
            if (timeout_id != this->timeout_id_) { return; }
            LOGV("timer", "Timeout expired: {}", timeout_id);

            ASSERT(posted_task_expiration_ != std::numeric_limits<TimeMs>::max(), "");
            posted_task_expiration_ = std::numeric_limits<TimeMs>::max();

            if (timeout_expiration_ == std::numeric_limits<TimeMs>::max()) {
                // cancelled timer
                // do nothing
            } else {
                const TimeMs now = parent_.get_time_();
                if (timeout_expiration_ <= now) {
                    timeout_expiration_ = std::numeric_limits<TimeMs>::max();
                    LOGV("timer", "Timeout Triggered: {}", timeout_id);
                    parent_.on_expired_(timeout_id_);
                } else {
                    const DurationMs remaining = timeout_expiration_ - now;
                    timeout_expiration_ = std::numeric_limits<TimeMs>::max();
                    Start(remaining, timeout_id);
                }
            }
        },
        sled::TimeDelta::Millis(duration_ms));
}

void
TaskQueueTimeoutFactory::TaskQueueTimeout::Stop()
{
    timeout_expiration_ = std::numeric_limits<TimeMs>::max();
}

}// namespace sled
