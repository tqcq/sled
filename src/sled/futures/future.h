#ifndef SLED_FUTURES_FUTURE_H
#define SLED_FUTURES_FUTURE_H

#include <exception>
#pragma once
#include "sled/exec/detail/invoke_result.h"
#include "sled/futures/internal/failure_handling.h"
#include "sled/futures/internal/promise.h"
#include "sled/lang/attributes.h"
#include "sled/log/log.h"
#include "sled/meta/type_traits.h"
#include "sled/synchronization/event.h"
#include "sled/synchronization/mutex.h"
#include "sled/task_queue/task_queue_base.h"
#include "sled/utility/forward_on_copy.h"
#include <atomic>
#include <list>

namespace sled {
namespace future_detail {
// template<typename F, typename... Args>
// struct is_invocable : std::is_constructible<std::function<void(Args...)>,
//                                             std::reference_wrapper<typename std::remove_reference<F>::type>> {};
//
// template<typename R, typename F, typename... Args>
// struct is_invocable_r : std::is_constructible<std::function<R(Args...)>,
//                                               std::reference_wrapper<typename std::remove_reference<F>::type>> {};
//
// template<bool cond, typename T = void>
// using enable_if_t = typename std::enable_if<cond, T>::type;

enum FutureState {
    kNotCompletedFuture = 0,
    kSuccessFuture      = 1,
    kFailedFuture       = 2,
};

SLED_EXPORT void IncrementFuturesUsage();
SLED_EXPORT void DecrementFuturesUsage();

template<typename T, typename FailureT>
struct FutureData {
    FutureData() { IncrementFuturesUsage(); }

    FutureData(const FutureData &)            = delete;
    FutureData(FutureData &&)                 = delete;
    FutureData &operator=(const FutureData &) = delete;
    FutureData &operator=(FutureData &&)      = delete;

    ~FutureData() { DecrementFuturesUsage(); }

    std::atomic_int state{kNotCompletedFuture};
    // sled::variant<sled::monostate, T, FailureT> value;
    sled::any value;
    std::list<std::function<void(const T &)>> success_callbacks;
    std::list<std::function<void(const FailureT &)>> failure_callbacks;
    sled::Mutex mutex_;
};
}// namespace future_detail

void SetDefaultScheduler(TaskQueueBase *scheduler) noexcept;
TaskQueueBase *GetDefaultScheduler() noexcept;

template<typename T, typename FailureT = failure::DefaultException>
class Future {
    static_assert(!std::is_same<T, void>::value, "Future<void, _> is not allowed. Use Future<bool, _> instead");
    static_assert(!std::is_same<FailureT, void>::value, "Future<_, void> is not allowed. Use Future<_, bool> instead");
    template<typename T2, typename FailureT2>
    friend class Future;
    friend class Promise<T, FailureT>;
    friend struct future_detail::FutureData<T, FailureT>;

public:
    using Value   = T;
    using Failure = FailureT;

    Future() noexcept = default;

    explicit Future(const Promise<T, FailureT> &promise) { data_ = promise.future().data_; }

    Future(const Future<T, FailureT> &) noexcept                         = default;
    Future(Future<T, FailureT> &&) noexcept                              = default;
    Future<T, FailureT> &operator=(const Future<T, FailureT> &) noexcept = default;
    Future<T, FailureT> &operator=(Future<T, FailureT> &&) noexcept      = default;
    ~Future()                                                            = default;

    // Future(const T &value) noexcept
    // {
    //     static_assert(!std::is_same<T, FailureT>::value, "T and FailureT must be different types");
    //     data_ = Future<T, FailureT>::Create().data_;
    //     FillSuccess(value);
    // }
    //
    // Future(T &&value) noexcept
    // {
    //     static_assert(!std::is_same<T, FailureT>::value, "T and FailureT must be different types");
    //     data_ = Future<T, FailureT>::Create().data_;
    //     FillSuccess(std::move(value));
    // }
    //
    // template<typename = EnableIfT<!std::is_same<T, FailureT>::value>>
    // Future(const FailureT &failure) noexcept
    // {
    //     static_assert(!std::is_same<T, FailureT>::value, "T and FailureT must be different types");
    //     data_ = Future<T, FailureT>::Create().data_;
    //     FillFailure(failure);
    // }

    bool operator==(const Future<T, FailureT> &other) const noexcept { return data_ == other.data_; }

    bool operator!=(const Future<T, FailureT> &other) const noexcept { return !operator==(other); }

    bool IsCompleted() const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        int value = data_->state.load(std::memory_order_acquire);
        return value == future_detail::kSuccessFuture || value == future_detail::kFailedFuture;
    }

    bool IsFailed() const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        return data_->state.load(std::memory_order_acquire) == future_detail::kFailedFuture;
    }

    bool IsSucceeded() const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        return data_->state.load(std::memory_order_acquire) == future_detail::kSuccessFuture;
    }

    bool IsValid() const noexcept { return static_cast<bool>(data_); }

    bool Wait(int64_t timeout_ms) const noexcept { return Wait(sled::TimeDelta::Millis(timeout_ms)); }

    bool Wait(sled::TimeDelta timeout = sled::Event::kForever) const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        if (IsCompleted()) { return true; }

        bool wait_forever         = timeout <= sled::TimeDelta::Zero();
        sled::TimeDelta wait_time = wait_forever ? sled::Event::kForever : timeout;

        auto event_ptr = std::make_shared<sled::Event>();
        OnComplete([event_ptr]() { event_ptr->Set(); });
        event_ptr->Wait(wait_time);

        return IsCompleted();
    }

    template<typename Dummy = void, typename = EnableIfT<std::is_copy_constructible<T>::value, Dummy>>
    T Result() const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        if (!IsCompleted()) Wait();
        if (IsSucceeded()) {
            try {
                // return sled::get<T>(data_->value);
                return sled::any_cast<T>(data_->value);
            } catch (...) {}
        }
        return T();
    }

    const T &ResultRef() const
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        if (!IsCompleted()) { Wait(); }
        // return sled::get<T>(data_->value);
        return sled::any_cast<T>(data_->value);
    }

    FailureT FailureReason() const
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        if (!IsCompleted()) { Wait(); }
        if (IsFailed()) {
            try {
                // return sled::get<FailureT>(data_->value);
                return sled::any_cast<FailureT>(data_->value);
            } catch (...) {}
        }
        return FailureT();
    }

    template<typename Func, typename = EnableIfT<IsInvocable<Func, T>::value>>
    Future<T, FailureT> OnSuccess(Func &&f) const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        bool call_it = false;
        {
            sled::MutexLock lock(&data_->mutex_);
            if (IsCompleted()) {
                call_it = IsSucceeded();
            } else {
                try {
                    data_->success_callbacks.emplace_back(std::forward<Func>(f));
                } catch (std::exception &e) {
                    return Future<T, FailureT>::Failed(future_detail::ExceptionFailure<FailureT>(e));
                } catch (...) {
                    return Future<T, FailureT>::Failed(future_detail::ExceptionFailure<FailureT>());
                }
            }
        }
        if (call_it) {
            try {
                // f(sled::get<T>(data_->value));
                f(sled::any_cast<T>(data_->value));
            } catch (...) {}
        }
        return Future<T, FailureT>(data_);
    }

    template<typename Func, typename = EnableIfT<IsInvocable<Func, FailureT>::value>>
    Future<T, FailureT> OnFailure(Func &&f) const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        bool call_it = false;
        {
            sled::MutexLock lock(&data_->mutex_);
            if (IsCompleted()) {
                call_it = IsFailed();
            } else {
                try {
                    data_->failure_callbacks.emplace_back(std::forward<Func>(f));
                } catch (std::exception &e) {
                    return Future<T, FailureT>::Failed(future_detail::ExceptionFailure<FailureT>(e));
                } catch (...) {
                    return Future<T, FailureT>::Failed(future_detail::ExceptionFailure<FailureT>());
                }
            }
        }
        if (call_it) {
            try {
                // f(sled::get<FailureT>(data_->value));
                f(sled::any_cast<FailureT>(data_->value));
            } catch (...) {}
        }
        return Future<T, FailureT>(data_);
    }

    template<typename Func, typename = EnableIfT<IsInvocable<Func>::value>>
    Future<T, FailureT> OnComplete(Func &&f) const noexcept
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        OnSuccess([f](const T &) noexcept { f(); });
        OnFailure([f](const FailureT &) noexcept { f(); });
        return Future<T, FailureT>(data_);
    }

    template<typename Func, typename = EnableIfT<IsInvocableR<bool, Func, T>::value>>
    Future<T, FailureT>
    Filter(Func &&f,
           const FailureT &rejected = failure::FailureFromString<FailureT>("Result wasn't good enough")) const noexcept
    {
        Future<T, FailureT> result = Future<T, FailureT>::Create();
        OnSuccess([result, f, rejected](const T &v) mutable noexcept {
            try {
                if (f(v)) {
                    result.FillSuccess(v);
                } else {
                    result.FillFailure(rejected);
                }
            } catch (const std::exception &e) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>(e));
            } catch (...) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>());
            }
        });
        OnFailure([result](const FailureT &failure) mutable noexcept { result.FillFailure(failure); });
        return result;
    }

    template<typename Func, typename U = InvokeResultT<Func, T>>
    Future<U, FailureT> Map(Func &&f) const noexcept
    {
        Future<U, FailureT> result = Future<U, FailureT>::Create();
        OnSuccess([result, f](const T &v) mutable noexcept {
            try {
                result.FillSuccess(f(v));
            } catch (const std::exception &e) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>(e));
            } catch (...) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>());
            }
        });
        OnFailure([result](const FailureT &failure) mutable noexcept { result.FillFailure(failure); });
        return result;
    }

    template<typename Func, typename OtherFailureT = InvokeResultT<Func, FailureT>>
    Future<T, OtherFailureT> MapFailure(Func &&f) const noexcept
    {
        Future<T, OtherFailureT> result = Future<T, OtherFailureT>::Create();
        OnSuccess([result](const T &v) mutable noexcept { result.FillSuccess(v); });
        OnFailure([result, f](const FailureT &failure) mutable noexcept {
            try {
                result.FillFailure(f(failure));
            } catch (const std::exception &e) {
                result.FillFailure(future_detail::ExceptionFailure<OtherFailureT>(e));
            } catch (...) {
                result.FillFailure(future_detail::ExceptionFailure<OtherFailureT>());
            }
        });
        return result;
    }

    template<typename Func, typename U = decltype(std::declval<InvokeResultT<Func, T>>().Result())>
    Future<U, FailureT> FlatMap(Func &&f) const noexcept
    {
        Future<U, FailureT> result = Future<U, FailureT>::Create();

        OnSuccess([result, f](const T &v) mutable noexcept {
            try {
                f(v).OnSuccess([result](const U &v) mutable noexcept { result.FillSuccess(v); })
                    .OnFailure([result](const FailureT &failure) mutable noexcept { result.FillFailure(failure); });
            } catch (const std::exception &e) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>(e));
            } catch (...) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>());
            }
        });
        OnFailure([result](const FailureT &failure) mutable noexcept { result.FillFailure(failure); });
        return result;
    }

    template<typename Func, typename U = decltype(std::declval<InvokeResultT<Func, T>().Result()>)>
    Future<U, FailureT> AndThen(Func &&f) const noexcept
    {
        return FlatMap([f](const T &) { return f(); });
    }

    template<typename T2, typename U = typename std::decay<T2>::type>
    Future<U, FailureT> AndThenValue(T2 &&value) const noexcept
    {
        Future<U, FailureT> result = Future<U, FailureT>::Create();
        auto forward_on_copy       = sled::MakeForwardOnCopy(std::forward<T2>(value));
        return Map([forward_on_copy](const T &) noexcept { return forward_on_copy.value(); });
    }

    Future<T, FailureT> Via(TaskQueueBase *task_queue) const noexcept
    {
        SLED_ASSERT(task_queue != nullptr, "TaskQueue is not valid");

        Future<T, FailureT> result = Future<T, FailureT>::Create();
        OnSuccess([result, task_queue](const T &v) mutable noexcept {
            task_queue->PostTask([result, v]() mutable noexcept { result.FillSuccess(v); });
        });
        OnFailure([result, task_queue](const FailureT &failure) mutable noexcept {
            task_queue->PostTask([result, failure]() mutable noexcept { result.FillFailure(failure); });
        });

        return result;
    }

    template<typename Func, typename = EnableIfT<IsInvocable<Func>::value>>
    static Future<T, FailureT> Async(Func &&f) noexcept
    {
        Future<T, FailureT> result = Future<T, FailureT>::Create();
        sled::GetDefaultScheduler()->PostTask([result, f]() mutable noexcept {
            try {
                result.FillSuccess(f());
            } catch (const std::exception &e) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>(e));
            } catch (...) {
                result.FillFailure(future_detail::ExceptionFailure<FailureT>());
            }
        });
        return result;
    }

    static Future<T, FailureT> AsyncValue(const T &value) noexcept
    {
        return Async([value]() { return value; });
    }

    static Future<typename std::decay<T>::type, FailureT> Successful(T &&value) noexcept
    {
        Future<typename std::decay<T>::type, FailureT> result
            = Future<typename std::decay<T>::type, FailureT>::Create();
        result.FillSuccess(std::forward<T>(value));
        return result;
    }

    static Future<T, FailureT> Successful(const T &value) noexcept
    {
        Future<T, FailureT> result = Future<T, FailureT>::Create();
        result.FillSuccess(value);
        return result;
    }

    static Future<T, FailureT> Successful() { return Future<T, FailureT>::Successful(T()); }

    static Future<T, FailureT> Failed(const FailureT &failure) noexcept
    {
        Future<T, FailureT> result = Future<T, FailureT>::Create();
        result.FillFailure(failure);
        return result;
    }

private:
    explicit Future(std::shared_ptr<future_detail::FutureData<T, FailureT>> other_data) { data_ = other_data; }

    inline static Future<T, FailureT> Create()
    {
        Future<T, FailureT> result;
        result.data_ = std::make_shared<future_detail::FutureData<T, FailureT>>();
        return result;
    }

    void FillSuccess(const T &value)
    {
        T copy = value;
        FillSuccess(std::move(copy));
    }

    void FillSuccess(T &&value)
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        if (future_detail::HasLastFailure()) {
            FailureT failure = future_detail::LastFailure<FailureT>();
            future_detail::InvalidateLastFailure();
            FillFailure(std::move(failure));
            return;
        }

        std::list<std::function<void(const T &)>> callbacks;
        {
            sled::MutexLock lock(&data_->mutex_);
            if (IsCompleted()) { return; }

            try {
                // data_->value.template emplace<T>(std::move(value));
                // data_->value.template emplace<T>(std::move(value));
                data_->value = std::move(value);
            } catch (...) {}
            data_->state.store(future_detail::kSuccessFuture, std::memory_order_release);
            callbacks                = std::move(data_->success_callbacks);
            data_->success_callbacks = std::list<std::function<void(const T &)>>();
            data_->failure_callbacks.clear();
        }

        for (const auto &f : callbacks) {
            try {
                // f(sled::get<T>(data_->value));
                f(sled::any_cast<T>(data_->value));
            } catch (...) {}
        }
    }

    void FillFailure(const FailureT &reason)
    {
        FailureT copy = reason;
        FillFailure(std::move(copy));
    }

    void FillFailure(FailureT &&reason)
    {
        SLED_ASSERT(data_ != nullptr, "Future is not valid");
        std::list<std::function<void(const FailureT &)>> callbacks;
        {
            sled::MutexLock lock(&data_->mutex_);
            if (IsCompleted()) { return; }
            try {
                // data_->value.template emplace<FailureT>(std::move(reason));
                // data_->value = std::move(reason);
                data_->value = std::move(reason);
            } catch (...) {}
            data_->state.store(future_detail::kFailedFuture, std::memory_order_release);
            callbacks                = std::move(data_->failure_callbacks);
            data_->failure_callbacks = std::list<std::function<void(const FailureT &)>>();
            data_->success_callbacks.clear();
        }

        for (const auto &f : callbacks) {
            try {
                // f(sled::get<FailureT>(data_->value));
                f(sled::any_cast<FailureT>(data_->value));
            } catch (...) {}
        }
    }

    std::shared_ptr<future_detail::FutureData<T, FailureT>> data_;
};

}// namespace sled

#endif// SLED_FUTURES_FUTURE_H
