#ifndef SLED_FUTURES_INTERNAL_PROMISE_H
#define SLED_FUTURES_INTERNAL_PROMISE_H

#pragma once

#include "failure_handling.h"
#include <memory>
#include <type_traits>

namespace sled {
template<typename T, typename FailureT>
class Future;

template<typename T, typename FailureT = failure::DefaultException>
class Promise final {
    static_assert(!std::is_same<T, void>::value, "Promise<void, _> is not allowed. Use Promise<bool, _> instead");
    static_assert(!std::is_same<FailureT, void>::value,
                  "Promise<_, void> is not allowed. Use Promise<_, bool> instead");

public:
    using Value                             = T;
    Promise()                               = default;
    Promise(const Promise &)                = default;
    Promise(Promise &&) noexcept            = default;
    Promise &operator=(const Promise &)     = default;
    Promise &operator=(Promise &&) noexcept = default;
    ~Promise()                              = default;

    Future<T, FailureT> GetFuture() const { return future_; };

    bool IsFilled() const noexcept { return future_.IsCompleted(); }

    void Failure(const FailureT &reason) const noexcept { return future_.FillFailure(reason); }

    void Success(const T &value) const noexcept { return future_.FillSuccess(value); }

    void Success(T &&value) const noexcept { return future_.FillSuccess(std::move(value)); }

private:
    mutable Future<T, FailureT> future_ = Future<T, FailureT>::Create();
};
}// namespace sled

#endif// SLED_FUTURES_INTERNAL_PROMISE_H
