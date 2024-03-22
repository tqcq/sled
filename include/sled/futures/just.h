#ifndef SLED_FUTURES_JUST_H
#define SLED_FUTURES_JUST_H
#include <utility>
#pragma once

namespace sled {
namespace futures {
template<typename T>
struct JustCell {
    T value;

    template<typename R>
    void Start(R receiver)
    {
        receiver.SetValue(value);
    }
};

template<typename T>
JustCell<T>
Just(T &&t)
{
    return {std::forward<T>(t)};
}

}// namespace futures
}// namespace sled
#endif//  SLED_FUTURES_JUST_H
