#pragma once
#ifndef SLED_FUTURES_STATE_H
#define SLED_FUTURES_STATE_H
#include "sled/futures/try.h"
#include "sled/synchronization/mutex.h"
#include <type_traits>

namespace sled {
template<typename T>
struct State {
public:
    enum InnerState {
        kNone,
        kTimeout,
        kDone,
    };

    static_assert(std::is_same<T, void>::value || std::is_copy_constructible<T>() || std::is_move_constructible<T>(),
                  "Must be copyable or movable or void");

    State() {}

    sled::Mutex &GetMutex() { return mutex_; }

    using ValueType = typename TryWrapper<T>::Type;
    sled::Mutex mutex_;
    ValueType value_ GUARDED_BY(mutex_);
    InnerState progress_ GUARDED_BY(mutex_) = kNone;
    std::function<void(std::function<void()>)> on_timeout_;
    std::function<void(ValueType &&)> on_then_;
};

}// namespace sled
#endif//  SLED_FUTURES_STATE_H
