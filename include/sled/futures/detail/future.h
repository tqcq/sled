#ifndef SLED_FUTURES_DETAIL_FUTURE_H
#define SLED_FUTURES_DETAIL_FUTURE_H
#include "sled/synchronization/mutex.h"
#include "traits.h"

namespace sled {
namespace detail {

namespace {
enum class State {
    kPending,
    kError,
    kValue,
};

template<typename T>
struct FutureState {

    sled::Mutex mutex;
    sled::ConditionVariable cv;
    State state = State::kPending;

    union {
        T value;
        std::exception_ptr error;
    };
};
}// namespace

template<typename R>
struct FutureOperation {
    std::shared_ptr<FutureState<typename R::result_t>> state;
    R receiver;
    mutable bool stopped = false;

    void Start()
    {
        if (stopped) { return; }
        sled::MutexLock lock(&state->mutex);
        state->cv.Wait(lock, [&] { return state->state != State::kPending; });
        if (state->state == State::kValue) {
            receiver.SetValue(std::move(state->value));
        } else {
            receiver.SetError(state->error);
        }
    }

    void Stop()
    {
        if (stopped) { return; }
        stopped = true;
        receiver.SetStopped();
    }
};

template<typename T>
struct FutureSender {
    using result_t = T;
    using this_type = FutureSender<T>;
    std::shared_ptr<FutureState<T>> state;

    template<typename R>
    FutureOperation<R> Connect(R receiver)
    {
        return {state, receiver};
    }

    template<typename Lazy>
    friend ContinueResultT<this_type, Lazy> operator|(this_type sender, Lazy lazy)
    {
        return lazy.Continue(sender);
    }
};

template<typename T>
struct Promise {
    std::shared_ptr<FutureState<T>> state;

    Promise() : state(std::make_shared<FutureState<T>>()) {}

    template<typename U = T>
    typename std::enable_if<std::is_convertible<U, T>::value>::type SetValue(U &&val)
    {
        sled::MutexLock lock(&state->mutex);
        state->value = std::forward<T>(val);
        state->state = State::kValue;
        state->cv.NotifyAll();
    }

    void SetError(std::exception_ptr e)
    {
        sled::MutexLock lock(&state->mutex);
        state->error = e;
        state->state = State::kError;
        state->cv.NotifyAll();
    }

    FutureSender<T> GetFuture() { return {state}; }
};

}// namespace detail
}// namespace sled
#endif// SLED_FUTURES_DETAIL_FUTURE_H
