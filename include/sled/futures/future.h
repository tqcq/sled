#pragma once
#include <exception>
#ifndef SLED_FUTURES_FUTURE_H
#define SLED_FUTURES_FUTURE_H

#include "sled/futures/state.h"
#include "sled/futures/try.h"
#include "sled/units/time_delta.h"
#include <type_traits>

namespace sled {
template<typename T>
class Promise;
template<typename T>
class Future;

namespace {
template<typename T>
struct IsFuture : std::false_type {
    typedef T Inner;
};

template<typename T>
struct IsFuture<Future<T>> : std::true_type {
    typedef T Inner;
};

template<typename F, typename... Args>
struct InvokeResult {
    using Type = decltype(std::declval<F>()(std::declval<Args>()...));
};

template<typename F, typename... Args>
struct CallableWith {
    template<typename T, typename = typename InvokeResult<F, Args...>::Type>
    static constexpr bool check(std::nullptr_t)
    {
        return true;
    }

    template<typename>
    static constexpr bool check(...)
    {
        return false;
    }

    static constexpr bool value = check(check<F>(nullptr));
};

template<bool is_try, typename F, typename... Args>
struct ArgResult {
    using Function = F;
    using Result = typename InvokeResult<F, Args...>::Type;

    static constexpr bool IsTry() { return is_try; }
};

template<typename T, typename F>
struct CallableResult {
    typedef typename std::conditional<
        CallableWith<F>::value,
        ArgResult<false, F>,
        typename std::conditional<
            CallableWith<F, T &&>::value,
            ArgResult<false, F, T &&>,
            typename std::conditional<CallableWith<F, T &>::value,
                                      ArgResult<false, F, T &>,
                                      typename std::conditional<CallableWith<F, Try<T> &&>::value,
                                                                ArgResult<true, F, Try<T> &&>,
                                                                ArgResult<true, F, Try<T> &>>::type>::type>::type>::type
        Arg;
    typedef IsFuture<typename Arg::Result> ReturnsFuture;
    typedef Future<typename ReturnsFuture::Inner> Return;
};
}// namespace

template<typename T>
class Future {
    using TimeoutCallback = std::function<void()>;

public:
    Future() = default;

    explicit Future(std::shared_ptr<State<T>> state) : state_(std::move(state)) {}

    Future(const Future &) = delete;

    Future &operator=(const Future &) = delete;
    Future(Future &&) = default;
    Future &operator=(Future &&) = default;

    bool Valid() const { return state_ != nullptr; }

    template<typename F, typename R = CallableResult<F, T>>
    auto Then(F &&func) -> typename R::Return
    {
        typedef typename R::Arg Arguments;
        return ThenImpl<F, R>(std::forward<F>(func), Arguments());
    }

    // for returns Future<T>
    template<typename F, typename R, typename... Args>
    typename std::enable_if<R::ReturnsFuture::value, typename R::Return>::type
    ThenImpl(F &&func, typename InvokeResult<F, Args...>::Type)
    {
        static_assert(sizeof...(Args) <= 1, "Then must take zero/one argument");
        typedef typename R::Return::Inner ValueType;
        Promise<ValueType> promise;

        auto next_future = promise.GetFuture();
        {
            auto weak_parent_state = std::weak_ptr<State<T>>(state_);
            next_future.SetOnTimeout([weak_parent_state = weak_parent_state](TimeoutCallback &&cb) {
                auto parent_state = weak_parent_state.lock();
                if (!parent_state) { return; }

                // set timeout
                {
                    MutexLock lock(&parent_state->mutex_);
                    if (parent_state->progress_ != State<T>::kNone) { return; }
                    parent_state->progress_ = State<T>::kTimeout;
                }
            });

            next_future.SetCallback([weak_parent_state = weak_parent_state,
                                     func = std::forward<std::decay<F>::type>(func),
                                     promise = std::move(promise)](typename TryWrapper<T>::Type &&t) {
                auto parent_state = weak_parent_state.lock();
                if (parent_state) {
                } else {
                }
            });
        }

        return std::move(next_future);
    }

    Future<T> &Via() { return *this; }

    Future<T> &OnTimeout(sled::TimeDelta duration, TimeoutCallback &&cb) { return *this; }

private:
    void SetCallback(TryWrapper<T> &&t);
    void SetOnTimeout(std::function<void(TimeoutCallback &&)> &&func);

    std::shared_ptr<State<T>> state_;
};
}// namespace sled
#endif//  SLED_FUTURES_FUTURE_H
