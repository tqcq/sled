#ifndef SLED_FUTURES_INTERNAL_FAILURE_HANDLING_H
#define SLED_FUTURES_INTERNAL_FAILURE_HANDLING_H

#pragma once
#include "sled/any.h"
#include <string>

namespace sled {
namespace failure {
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

}// namespace failure

namespace detail {
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
}// namespace detail

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
        detail::SetLastFailure(std::move(failure_));
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
