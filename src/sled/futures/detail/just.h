#ifndef SLED_FUTURES_DETAIL_JUST_H
#define SLED_FUTURES_DETAIL_JUST_H

#include "traits.h"
#include <memory>

namespace sled {
namespace detail {

template<typename T, typename R>
struct JustOperation {
    T value;
    R receiver;

    void Start() { receiver.SetValue(std::forward<T>(value)); }

    void Stop() { receiver.SetStopped(); }
};

template<typename T>
struct JustSender {
    using result_t  = T;
    using this_type = JustSender<T>;
    T value;

    template<typename R>
    JustOperation<T, R> Connect(R receiver)
    {
        return {std::forward<T>(value), receiver};
    }

    template<typename Lazy>
    friend ContinueResultT<this_type, Lazy> operator|(this_type sender, Lazy lazy)
    {
        return lazy.Continue(sender);
    }
};

template<typename T>
JustSender<T>
Just(T &&value)
{
    return {std::forward<T>(value)};
}

}// namespace detail
}// namespace sled
#endif//  SLED_FUTURES_DETAIL_JUST_H
