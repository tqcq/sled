
#ifndef SLED_FUTURES_THEN_H
#define SLED_FUTURES_THEN_H
#include <exception>
#include <utility>
#pragma once

namespace sled {
namespace futures {

template<typename S, typename F>
struct ThenCell {
    S sender;
    F func;

    // T value;

    template<typename R>
    void Start(R receiver)
    {
        sender.Start();
    }

    template<typename U>
    void SetValue(U &&value)
    {}

    void SetError(std::exception_ptr err) {}
};

template<typename S, typename F>
ThenCell<S, F>
Then(S sender, F &&func)
{
    return {std::forward<S>(sender), std::forward<F>(func)};
}

}// namespace futures
}// namespace sled
#endif//  SLED_FUTURES_THEN_H
