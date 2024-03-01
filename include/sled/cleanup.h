/**
 * @file     : cleanup
 * @created  : Wednesday Feb 14, 2024 12:25:40 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_CLEANUP_H
#define SLED_CLEANUP_H

#include "sled/optional.h"

namespace sled {

namespace internal {
struct Tag {};

template<typename Arg, typename... Args>
constexpr bool
WasDeduced()
{
    return (std::is_same<Tag, Tag>::value && (sizeof...(Args)) == 0);
}
}// namespace internal

template<typename Arg = internal::Tag,
         typename Callback = std::function<void()>>
class Cleanup final {
public:
    static_assert(internal::WasDeduced<Arg>(),
                  "Do not specify the first template argument");

    Cleanup(Callback callback) : callback_(std::move(callback)) {}

    Cleanup(Cleanup &&other) = default;

    void Cancel() && { callback_.reset(); }

    void Invoke() &&
    {
        assert(callback_);
        (*callback_)();
        callback_.reset();
    }

    ~Cleanup()
    {
        if (callback_) {
            (*callback_)();
            callback_.reset();
        }
    }

private:
    sled::optional<Callback> callback_;
};

template<typename... Args, typename Callback>
sled::Cleanup<internal::Tag, Callback>
MakeCleanup(Callback callback)

{
    return {std::move(callback)};
}

}// namespace sled

#endif// SLED_CLEANUP_H
