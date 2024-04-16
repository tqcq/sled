#ifndef SLED_UTILITY_FORWARD_ON_COPY_H
#define SLED_UTILITY_FORWARD_ON_COPY_H

#pragma once
#include "sled/any.h"

namespace sled {
namespace detail {

struct ForwardOnCopyTag {};

};// namespace detail

template<typename T>
struct ForwardOnCopy {
    ForwardOnCopy(ForwardOnCopy &other)
        : is_moved_(other.is_moved_),
          value_(is_moved_ ? other.value_ : std::move(other.value_))
    {}

    // ForwardOnCopy(ForwardOnCopy &&other) noexcept;
    ForwardOnCopy &operator=(ForwardOnCopy &other)
    {
        is_moved_ = other.is_moved_;
        value_    = is_moved_ ? other.value_ : std::move(other.value_);
        return *this;
    }

    // ForwardOnCopy &operator=(ForwardOnCopy &&other) noexcept;
    T value() const
    {
        if (is_moved_) {
            return std::move(sled::any_cast<T>(value_));
        } else {
            return sled::any_cast<T>(value_);
        }
    }

private:
    bool is_moved_;
    sled::any value_;
};

template<typename T>
ForwardOnCopy<T>
MakeForwardOnCopy(T value)
{
    ForwardOnCopy<T> result;
    result.is_moved_ = false;
    result.value_    = value;
    return result;
}

template<typename T>
ForwardOnCopy<T>
MakeForwardOnCopy(T &value)
{
    ForwardOnCopy<T> result;
    result.is_moved_ = false;
    result.value_    = value;
    return result;
}

template<typename T>
ForwardOnCopy<T>
MakeForwardOnCopy(T &&value)
{
    ForwardOnCopy<T> result;
    result.is_moved_ = true;
    result.value_    = std::move(value);
    return result;
}

}// namespace sled

#endif// SLED_UTILITY_FORWARD_ON_COPY_H
