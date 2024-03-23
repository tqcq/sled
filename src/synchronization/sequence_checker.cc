#include "sled/synchronization/sequence_checker.h"

namespace sled {
SequenceChecker::SequenceChecker(InitialState initial_state)
    : attached_(initial_state),
      valid_thread_(pthread_self()),
      valid_queue_(TaskQueueBase::Current())
{}

bool
SequenceChecker::IsCurrent() const
{
    const TaskQueueBase *const current_queue = TaskQueueBase::Current();
    const pthread_t current_thread = pthread_self();
    MutexLock lock(&mutex_);
    if (!attached_) {
        attached_ = true;
        valid_thread_ = current_thread;
        valid_queue_ = current_queue;
        return true;
    }

    if (valid_queue_) { return valid_queue_ == current_queue; }
    return pthread_equal(valid_thread_, current_thread);
}

void
SequenceChecker::Detach()
{
    MutexLock lock(&mutex_);
    attached_ = false;
}

std::string
SequenceChecker::ExpectationToString() const
{
    const TaskQueueBase *const current_queue = TaskQueueBase::Current();
    const pthread_t current_thread = pthread_self();

    MutexLock lock(&mutex_);
    if (!attached_) { return "Checker currently not attached."; }

    std::stringstream ss;
    ss << "# Expected: TQ: " << valid_queue_ << " Thread: " << reinterpret_cast<const void *>(valid_thread_)
       << std::endl;
    ss << "# Current: TQ: " << current_queue << " Thread: " << reinterpret_cast<const void *>(current_thread)
       << std::endl;
    if ((valid_queue_ || current_queue) && valid_queue_ != current_queue) {
        ss << "# Mismatched TaskQueue" << std::endl;
    } else if (valid_thread_ != current_thread) {
        ss << "# Mismatched Thread" << std::endl;
    }

    return ss.str();
}
}// namespace sled
