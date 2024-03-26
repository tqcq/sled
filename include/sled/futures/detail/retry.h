#ifndef SLED_FUTURES_DETAIL_RETRY_H
#define SLED_FUTURES_DETAIL_RETRY_H

#include "sled/log/log.h"
#include "sled/synchronization/mutex.h"
#include "traits.h"
#include <memory>

namespace sled {
namespace detail {

namespace {
struct RetryState {
    enum State { kPending, kDone, kRetry };

    sled::Mutex mutex;
    sled::ConditionVariable cv;
    int retry_count = 0;
    State state = kPending;
};
}// namespace

template<typename R>
struct RetryReceiver {
    std::shared_ptr<RetryState> state;
    R receiver;
    bool stopped = false;

    template<typename U>
    void SetValue(U &&val)
    {
        {
            sled::MutexLock lock(&state->mutex);
            if (stopped) { return; }
            state->state = RetryState::kDone;
            state->cv.NotifyAll();
        }
        receiver.SetValue(std::forward<U>(val));
    }

    void SetError(std::exception_ptr e)
    {
        // notify
        {
            sled::MutexLock lock(&state->mutex);
            if (stopped) { return; }
            if (state->retry_count > 0) {
                --state->retry_count;
                state->state = RetryState::kRetry;
                return;
            } else {
                state->state = RetryState::kDone;
                state->cv.NotifyAll();
            }
        }
        receiver.SetError(e);
    }

    void SetStopped()
    {
        {
            sled::MutexLock lock(&state->mutex);
            if (stopped) { return; }
            stopped = true;
            state->state = RetryState::kDone;
            state->cv.NotifyAll();
        }
        receiver.SetStopped();
    }
};

template<typename S, typename R>
struct RetryOperation {
    int retry_count;
    std::shared_ptr<RetryState> state;
    ConnectResultT<S, R> op;

    void Start()
    {
        {
            sled::MutexLock lock(&state->mutex);
            state->retry_count = retry_count;
            state->state = RetryState::kPending;
        }
        do {
            op.Start();
            sled::MutexLock lock(&state->mutex);
            state->cv.Wait(lock, [this] { return state->state != RetryState::kPending; });
            if (state->state == RetryState::kDone) { break; }
            state->state = RetryState::kPending;
        } while (true);
    }

    void Stop() { op.Stop(); }
};

template<typename S>
struct RetrySender {
    using result_t = typename S::result_t;
    using this_type = RetrySender<S>;
    S sender;
    int retry_count;

    template<typename R>
    RetryOperation<S, RetryReceiver<R>> Connect(R receiver)
    {
        auto state = std::make_shared<RetryState>();
        auto op = sender.Connect(RetryReceiver<R>{state, receiver});
        return {retry_count, state, op};
    }

    template<typename Lazy>
    friend ContinueResultT<this_type, Lazy> operator|(this_type sender, Lazy lazy)
    {
        return lazy.Continue(sender);
    }
};

template<typename S>
RetrySender<S>
Retry(S sender, int retry_count)
{
    return {sender, retry_count};
}

struct RetryLazy {
    int retry_count;

    template<typename S>
    RetrySender<S> Continue(S sender) const
    {
        return {sender, retry_count};
    }
};

inline RetryLazy
Retry(int retry_count)
{
    return {retry_count};
}

}// namespace detail
}// namespace sled
#endif//  SLED_FUTURES_DETAIL_RETRY_H
