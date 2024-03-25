#ifndef SLED_FUTURES_DETAIL_BASE_FUTURE_H
#define SLED_FUTURES_DETAIL_BASE_FUTURE_H
#include "sled/any.h"
#include "sled/synchronization/mutex.h"
#include <memory>
#include <type_traits>

namespace sled {

template<typename T>
class Promise;

template<typename T>
class FutureState {
public:
    T Get()
    {
        sled::MutexLock lock(&mutex_);
        cv_.Wait(&mutex_, [this]() { return done_; });
        return sled::any_cast<T>(value_);
    }

    void SetError(std::exception_ptr e)
    {
        sled::MutexLock lock(&mutex_);
        value_ = e;
        done_ = true;
    };

    template<typename U = T>
    typename std::enable_if<!std::is_void<T>::value && std::is_convertible<U, T>::value>::type SetValue(U &&value)
    {
        sled::MutexLock lock(&mutex_);
        value_ = static_cast<T>(std::forward<U>(value));
        done_ = true;
    }

    template<typename U = T>
    typename std::enable_if<std::is_void<T>::value>::type SetValue(U &&value)
    {
        sled::MutexLock lock(&mutex_);
        done_ = true;
    }

private:
    sled::Mutex mutex_;
    sled::ConditionVariable cv_;
    sled::any value_;
    bool done_{false};
};

template<typename T>
class BaseFuture {
public:
    template<typename T>
    T Get() const
    {
        return state_->Get();
    }

private:
    std::shared_ptr<FutureState<T>> state_;
};

}// namespace sled
#endif// SLED_FUTURES_DETAIL_BASE_FUTURE_H
