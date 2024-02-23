#include "sled/synchronization/event.h"

namespace sled {
constexpr TimeDelta Event::kForever;

Event::Event() : Event(false, false) {}

Event::Event(bool manual_reset, bool initially_signaled)
    : is_manual_reset_(manual_reset),
      event_status_(initially_signaled)
{}

Event::~Event() {}

void
Event::Set()
{
    MutexLock lock(&mutex_);
    event_status_ = true;
    cv_.NotifyAll();
}

void
Event::Reset()
{
    MutexLock lock(&mutex_);
    event_status_ = false;
}

bool
Event::Wait(TimeDelta give_up_after, TimeDelta warn_after)
{
    MutexLock lock(&mutex_);
    bool wait_success =
        cv_.WaitFor(&mutex_, give_up_after, [&] { return event_status_; });
    if (!wait_success) { return false; }

    if (!is_manual_reset_) { event_status_ = false; }
    return true;
}

}// namespace sled
