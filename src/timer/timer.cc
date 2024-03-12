#include "sled/timer/timer.h"

namespace sled {
namespace {
TimeoutID
MakeTimeoutId(TimerID timer_id, TimerGeneration generation)
{
    return TimeoutID(static_cast<uint64_t>((timer_id << 32) | generation));
}
}// namespace

Timer::Timer(TimerID id,
             const std::string &name,
             OnExpired on_expired,
             UnregisterHandler unregister_handler,
             std::unique_ptr<Timeout> timeout)
    : id_(id),
      name_(name),
      on_expired_(on_expired),
      unregister_handler_(unregister_handler),
      timeout_(std::move(timeout))
{}

Timer::~Timer()
{
    Stop();
    unregister_handler_();
}

void
Timer::Start()
{
    expiration_count_ = 0;

    if (!is_running()) {
        is_running_ = true;
        generation_ = TimerGeneration(generation_ + 1);
        timeout_->Start(duration_, MakeTimeoutId(id_, generation_));
    } else {
        generation_ = TimerGeneration(generation_ + 1);
        timeout_->Restart(duration_, MakeTimeoutId(id_, generation_));
    }
}

void
Timer::Stop()
{
    if (is_running()) {
        timeout_->Stop();
        expiration_count_ = 0;
        is_running_ = false;
    }
}

void
Timer::Trigger(TimerGeneration generation)
{
    if (!is_running_ || generation != generation_) { return; }
    ++expiration_count_;
    is_running_ = false;
    // if max_restarts > exppiration_count_ then restart
    {
        is_running_ = true;
        generation_ = TimerGeneration(generation_ + 1);
        timeout_->Start(duration_, MakeTimeoutId(id_, generation_));
    }

    sled::optional<DurationMs> new_duration = on_expired_();
    if (new_duration.has_value() && new_duration != duration_) {
        duration_ = new_duration.value();
        if (is_running_) {
            timeout_->Stop();
            generation_ = TimerGeneration(generation_ + 1);
            timeout_->Start(duration_, MakeTimeoutId(id_, generation_));
        }
    }
}

std::unique_ptr<Timer>
TimerManager::CreateTimer(const std::string &name, Timer::OnExpired on_expired)
{
    next_id_ = TimerID(next_id_ + 1);
    TimerID id = next_id_;

    std::unique_ptr<Timeout> timeout =
        timeout_creator_(sled::TaskQueueBase::DelayPrecision::kHigh);
    auto timer = std::unique_ptr<Timer>(new Timer(
        id, name, std::move(on_expired),
        /* ungrgister_handler=*/[this, id]() { timers_.erase(id); },
        std::move(timeout)));
    timers_[id] = timer.get();
    return timer;
}

void
TimerManager::HandleTimeout(TimeoutID id)
{
    TimerID timer_id = id >> 32;
    TimerGeneration generation = id & 0xffffffff;
    auto it = timers_.find(timer_id);
    if (it != timers_.end()) { it->second->Trigger(generation); }
}
}// namespace sled
