#ifndef SLED_FUTURES_INTERNAL_FAILURE_HANDLING_H
#define SLED_FUTURES_INTERNAL_FAILURE_HANDLING_H

#pragma once
#include "sled/any.h"
#include <functional>
#include <string>

namespace sled {
namespace failure {
namespace {
template<typename F, typename... Args>
struct is_invocable : std::is_constructible<std::function<void(Args...)>,
                                            std::reference_wrapper<typename std::remove_reference<F>::type>> {};

}// namespace

template<typename FailureT>
inline FailureT
FailureFromString(std::string &&)
{
    return FailureT();
}

template<typename FailureT>
inline FailureT
FailureFromString(const std::string &str)
{
    std::string copy = str;
    return FailureFromString<FailureT>(std::move(copy));
}

template<>
inline std::string
FailureFromString<std::string>(std::string &&str)
{
    return std::move(str);
}

template<typename FailureT,
         typename = typename std::enable_if<std::is_constructible<FailureT, std::string>::value>::type>
inline FailureT
FailureFromString(std::string &&str)
{
    return FailureT(std::move(str));
}

}// namespace failure

namespace future_detail {
bool HasLastFailure() noexcept;
void InvalidateLastFailure() noexcept;
const sled::any &LastFailureAny() noexcept;
void SetLastFailure(const sled::any &) noexcept;

template<typename FailureT>
inline FailureT
LastFailure() noexcept
{
    if (!HasLastFailure()) { return FailureT(); }
    try {
        return sled::any_cast<FailureT>(LastFailureAny());
    } catch (...) {
        return FailureT();
    }
}

template<typename FailureT>
inline void
SetLastFailure(const FailureT &failure) noexcept
{
    SetLastFailure(sled::any(failure));
}

template<typename FailureT>
FailureT
ExceptionFailure(const std::exception &e)
{
    return failure::FailureFromString<FailureT>(e.what());
}

template<typename FailureT>
FailureT
ExceptionFailure()
{
    return failure::FailureFromString<FailureT>("Exception");
}
}// namespace future_detail

template<typename T, typename FailureT>
class Future;

template<typename FailureT>
struct WithFuture {
    explicit WithFuture(const FailureT &f = FailureT()) noexcept : failure_(f) {}

    explicit WithFuture(FailureT &&f) noexcept : failure_(std::move(f)) {}

    template<typename... Args>
    explicit WithFuture(Args &&...args) noexcept : failure_(std::forward<Args>(args)...)
    {}

    template<typename T>
    operator T() const noexcept
    {
        future_detail::SetLastFailure(std::move(failure_));
        return T();
    }

    template<typename T>
    operator Future<T, FailureT>() noexcept
    {
        Future<T, FailureT> result = Future<T, FailureT>::Create();
        result.Faillure(failure_);
        return result;
    }

private:
    FailureT failure_;
};

}// namespace sled

#endif// SLED_FUTURES_INTERNAL_FAILURE_HANDLING_H
