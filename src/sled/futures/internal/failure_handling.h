#ifndef SLED_FUTURES_INTERNAL_FAILURE_HANDLING_H
#define SLED_FUTURES_INTERNAL_FAILURE_HANDLING_H

#pragma once
#include "sled/any.h"
#include "sled/meta/type_traits.h"
#include <string>

namespace sled {
namespace failure {

class DefaultException : std::exception {
public:
    inline DefaultException() = default;

    inline DefaultException(const std::string &message) : message_(message) {}

    inline DefaultException(const char *message) : message_(message) {}

    ~DefaultException() noexcept override = default;

    const char *what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

template<typename FailureT, typename = EnableIfT<!std::is_constructible<FailureT, std::string>::value>>
inline FailureT
FailureFromString(std::string &&str)
{
    return FailureT();
}

template<typename FailureT, typename = EnableIfT<std::is_constructible<FailureT, const std::string &>::value>>
inline FailureT
FailureFromString(const std::string &str)
{
    std::string copy = str;
    return FailureFromString<FailureT>(std::move(copy));
}

template<>
inline std::string
FailureFromString<std::string>(const std::string &str)
{
    return str;
}

template<>
inline DefaultException
FailureFromString<DefaultException>(const std::string &str)
{
    return DefaultException(str);
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
