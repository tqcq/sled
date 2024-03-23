#include "sled/timer/task_queue_timeout.h"
#include "sled/log/log.h"
#include "sled/task_queue/pending_task_safety_flag.h"
#include "sled/units/time_delta.h"

namespace sled {
TaskQueueTimeoutFactory::TaskQueueTimeout::TaskQueueTimeout(TaskQueueTimeoutFactory &parent,
                                                            sled::TaskQueueBase::DelayPrecision precision)
    : parent_(parent),
      precision_(precision),
      safety_flag_(PendingTaskSafetyFlag::Create())
{}

TaskQueueTimeoutFactory::TaskQueueTimeout::~TaskQueueTimeout()
{
    SLED_DCHECK_RUN_ON(&parent_.thread_checker_);
    safety_flag_->SetNotAlive();
}

void
TaskQueueTimeoutFactory::TaskQueueTimeout::Start(DurationMs duration_ms, TimeoutID timeout_id)
{
    SLED_DCHECK_RUN_ON(&parent_.thread_checker_);
    ASSERT(timeout_expiration_ == std::numeric_limits<TimeMs>::max(), "");
    timeout_expiration_ = parent_.get_time_() + duration_ms;
    timeout_id_ = timeout_id;

    if (timeout_expiration_ >= posted_task_expiration_) { return; }
    if (posted_task_expiration_ != std::numeric_limits<TimeMs>::max()) {
        LOGV("timer",
             "New timeout duration is less than scheduled - "
             "ghosting old delayed task");
        safety_flag_->SetNotAlive();
        safety_flag_ = PendingTaskSafetyFlag::Create();
    }

    posted_task_expiration_ = timeout_expiration_;
    auto safety_flag = safety_flag_;

    parent_.task_queue_.PostDelayedTaskWithPrecision(
        precision_,
        SafeTask(safety_flag_,
                 [timeout_id, this]() {
                     LOGV("timer", "Timeout expired: {}", timeout_id);
                     SLED_DCHECK_RUN_ON(&parent_.thread_checker_);
                     DCHECK(posted_task_expiration_ != std::numeric_limits<TimeMs>::max(), "");
                     posted_task_expiration_ = std::numeric_limits<TimeMs>::max();

                     if (timeout_expiration_ == std::numeric_limits<TimeMs>::max()) {
                         // cancelled timer
                         // do nothing
                     } else {
                         const TimeMs now = parent_.get_time_();
                         const DurationMs remaining = timeout_expiration_ - now;
                         bool is_expired = timeout_expiration_ <= now;

                         timeout_expiration_ = std::numeric_limits<TimeMs>::max();

                         if (!is_expired) {
                             // continue wait
                             Start(remaining, timeout_id);
                         } else {
                             LOGV("timer", "Timeout Triggered: {}", timeout_id);
                             parent_.on_expired_(timeout_id_);
                         }
                     }
                 }),
        sled::TimeDelta::Millis(duration_ms));
}

void
TaskQueueTimeoutFactory::TaskQueueTimeout::Stop()
{

    SLED_DCHECK_RUN_ON(&parent_.thread_checker_);
    timeout_expiration_ = std::numeric_limits<TimeMs>::max();
}

}// namespace sled
