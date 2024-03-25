#ifndef SLED_FUTURES_DETAIL_BASE_FUTURE_H
#define SLED_FUTURES_DETAIL_BASE_FUTURE_H
#include "sled/log/log.h"
#include "sled/optional.h"
#include "sled/synchronization/mutex.h"
#include <memory>
#include <type_traits>

namespace sled {

namespace {
enum class State {
    kCancel,
    kPending,
    kTimeout,
    kError,
    kValue,
};
};

template<typename T>
struct FutureState {
    mutable sled::Mutex mutex;
    mutable sled::ConditionVariable cond_var;

    sled::optional<T> value;
    std::exception_ptr error;

    State state = State::kPending;

    void AssertHasValue() const { ASSERT(state == State::kValue, "can't find value"); }

    void AssertHasError() const { ASSERT(state == State::kError, "can't find error"); }

    void AssertHasTimeout() const { ASSERT(state == State::kTimeout, "can't find timeout"); }

    void Wait(sled::MutexLock *lock_ptr = nullptr) const
    {
        if (lock_ptr) {
            if (state != State::kPending) { return; }
            cond_var.Wait(*lock_ptr, [this] { return state != State::kPending; });
        } else {
            sled::MutexLock lock(&mutex);
            if (state != State::kPending) { return; }
            cond_var.Wait(lock, [this] { return state != State::kPending; });
        }
    }

    void SetError(std::exception_ptr e)
    {
        sled::MutexLock lock(&mutex);
        if (state == State::kCancel) { return; }
        ASSERT(state == State::kPending, "state must be kPending");
        error = std::move(e);
        state = State::kError;
        cond_var.NotifyAll();
    }

    void SetTimeout()
    {
        sled::MutexLock lock(&mutex);
        if (state == State::kCancel) { return; }
        ASSERT(state == State::kPending, "state must be kPending");
        state = State::kTimeout;
        cond_var.NotifyAll();
    }

    template<typename U = T>
    typename std::enable_if<std::is_convertible<U, T>::value>::type SetValue(U &&val)
    {
        sled::MutexLock lock(&mutex);
        if (state == State::kCancel) { return; }
        ASSERT(state == State::kPending, "state must be kPending");
        value = std::forward<U>(val);
        state = State::kValue;
        cond_var.NotifyAll();
    }
};

template<>
struct FutureState<void> {
    mutable sled::Mutex mutex;
    mutable sled::ConditionVariable cond_var;

    std::exception_ptr error;

    State state = State::kPending;

    void AssertHasValue() const { ASSERT(state == State::kValue, "can't find value"); }

    void AssertHasError() const { ASSERT(state == State::kError, "can't find error"); }

    void AssertHasTimeout() const { ASSERT(state == State::kTimeout, "can't find timeout"); }

    void Wait(sled::MutexLock *lock_ptr = nullptr) const
    {
        if (lock_ptr) {
            if (state != State::kPending) { return; }
            cond_var.Wait(*lock_ptr, [this] { return state != State::kPending; });
        } else {
            sled::MutexLock lock(&mutex);
            if (state != State::kPending) { return; }
            cond_var.Wait(lock, [this] { return state != State::kPending; });
        }
    }

    void SetTimeout()
    {
        sled::MutexLock lock(&mutex);
        if (state == State::kCancel) { return; }
        ASSERT(state == State::kPending, "state must be kPending");
        state = State::kTimeout;
        cond_var.NotifyAll();
    }

    void SetError(std::exception_ptr e)
    {
        sled::MutexLock lock(&mutex);
        if (state == State::kCancel) { return; }
        ASSERT(state == State::kPending, "state must be kPending");
        error = std::move(e);
        state = State::kError;
        cond_var.NotifyAll();
    }

    void SetValue()
    {
        sled::MutexLock lock(&mutex);
        if (state == State::kCancel) { return; }
        ASSERT(state == State::kPending, "state must be kPending");
        state = State::kValue;
        cond_var.NotifyAll();
    }
};

template<typename T>
class Future {
public:
    // using ValueType = typename std::remove_reference<T>::type;
    Future(std::shared_ptr<FutureState<T>> state) : state_(std::move(state)) {}

    T Get() const &
    {
        sled::MutexLock lock(&state_->mutex);
        state_->Wait(&lock);
        state_->AssertHasValue();
        return state_->value.value();
    }

    T &Get() &
    {
        sled::MutexLock lock(&state_->mutex);
        state_->Wait(&lock);
        state_->AssertHasValue();
        return state_->value.value();
    }

    T &&Get() &&
    {
        sled::MutexLock lock(&state_->mutex);
        state_->Wait(&lock);
        state_->AssertHasValue();
        return std::move(state_->value.value());
    }

private:
    std::shared_ptr<FutureState<T>> state_;
};

template<>
class Future<void> {
public:
    Future(std::shared_ptr<FutureState<void>> state) : state_(std::move(state)) {}

    void Wait() const { state_->Wait(); }

    void Get() const { Wait(); }

protected:
    std::shared_ptr<FutureState<void>> state_;
};

template<typename T>
class Promise {
public:
    Promise() : state_(new FutureState<T>()) {}

    Future<T> GetFuture() const { return Future<T>(state_); }

    template<typename U = T>
    typename std::enable_if<!std::is_void<U>::value && std::is_convertible<U, T>::value>::type SetValue(U &&val)
    {
        state_->SetValue(val);
    }

    template<typename U = T>
    typename std::enable_if<std::is_void<U>::value>::type SetValue()
    {
        state_->SetValue();
    }

    void SetError(std::exception_ptr e) { state_->SetError(e); }

    void SetTimeout() { state_->SetTimeout(); }

private:
    std::shared_ptr<FutureState<T>> state_;
};

}// namespace sled
#endif// SLED_FUTURES_DETAIL_BASE_FUTURE_H
