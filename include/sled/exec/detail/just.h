#ifndef SLED_EXEC_DETAIL_JUST_H
#define SLED_EXEC_DETAIL_JUST_H
#pragma once

#include <iostream>
#include <utility>

namespace sled {

template<typename TReceiver, typename T>
struct JustOperation {
    TReceiver receiver;
    T value;

    void Start() { receiver.SetValue(value); }
};

template<typename T>
struct JustSender {
    using result_t = T;
    T value;

    template<typename TReceiver>
    JustOperation<TReceiver, T> Connect(TReceiver &&receiver)
    {
        return {std::forward<TReceiver>(receiver), std::forward<T>(value)};
    }
};

template<typename T>
JustSender<T>
Just(T &&t)
{
    return {std::forward<T>(t)};
}

}// namespace sled

#endif// SLED_EXEC_DETAIL_JUST_H
