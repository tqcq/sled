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

    void Start() { receiver.SetValue(std::move(value)); }

    void Stop() { receiver.SetStopped(); }
};

template<typename T>
struct JustSender {
    using result_t = T;
    T value;

    template<typename R>
    JustOperation<T, R> Connect(R receiver)
    {
        return {value, receiver};
    }
};

template<typename T>
JustSender<T>
Just(T value)
{
    return {value};
}

}// namespace detail
}// namespace sled
#endif//  SLED_FUTURES_DETAIL_JUST_H
