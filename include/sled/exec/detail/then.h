#ifndef SLED_EXEC_DETAIL_THEN_H
#define SLED_EXEC_DETAIL_THEN_H
#pragma once

#include "traits.h"
#include <exception>
#include <functional>

namespace sled {
template<typename TReceiver, typename F>
struct ThenReceiver {
    TReceiver receiver;
    F func;

    template<typename T>
    void SetValue(T &&value)
    {
        try {
            receiver.SetValue(func(std::forward<T>(value)));
        } catch (...) {
            receiver.SetError(std::current_exception());
        }
    }

    void SetError(std::exception_ptr err) { receiver.SetError(err); }

    void SetStopped() { receiver.SetStopped(); }
};

template<typename TSender, typename TReceiver, typename F>
struct ThenOperation {
    ConnectResultT<TSender, ThenReceiver<TReceiver, F>> op;

    void Start() { op.Start(); }
};

template<typename TSender, typename F>
struct ThenSender {
    using S = typename std::remove_cv<typename std::remove_reference<TSender>::type>::type;
    using result_t = typename eggs::invoke_result_t<F, SenderResultT<S>>;
    S sender;
    F func;

    template<typename TReceiver>
    ThenOperation<TSender, TReceiver, F> Connect(TReceiver &&receiver)
    {
        return {sender.Connect(ThenReceiver<TReceiver, F>{std::forward<TReceiver>(receiver), func})};
    }
};

template<typename TSender, typename F>
ThenSender<TSender, F>
Then(TSender &&sender, F &&func)
{
    return {std::forward<TSender>(sender), std::forward<F>(func)};
}

}// namespace sled
#endif// SLED_EXEC_DETAIL_THEN_H
