#ifndef SLED_FUTURES_DETAIL_THEN_H
#define SLED_FUTURES_DETAIL_THEN_H

#include "traits.h"
#include <memory>

namespace sled {
namespace detail {

template<typename R, typename F>
struct ThenReceiver {
    R receiver;
    F func;
    bool stopped = false;

    template<typename U>
    void SetValue(U &&val)
    {
        if (stopped) { return; }
        try {
            receiver.SetValue(func(std::forward<U>(val)));
        } catch (...) {
            SetError(std::current_exception());
        }
    }

    void SetError(std::exception_ptr e)
    {
        if (stopped) { return; }
        receiver.SetError(e);
    }

    void SetStopped()
    {
        if (stopped) { return; }
        stopped = true;
        receiver.SetStopped();
    }
};

template<typename S, typename R>
struct ThenOperation {
    ConnectResultT<S, R> op;

    void Start() { op.Start(); }

    void Stop() { op.Stop(); }
};

template<typename S, typename F>
struct ThenSender {
    using result_t = invoke_result_t<F, typename S::result_t>;
    S sender;
    F func;

    template<typename R>
    ThenOperation<S, ThenReceiver<R, F>> Connect(R receiver)
    {
        return {sender.Connect(ThenReceiver<R, F>{receiver, func})};
    }
};

template<typename S, typename F>
ThenSender<S, F>
Then(S sender, F &&func)
{
    return {sender, std::forward<F>(func)};
}

}// namespace detail
}// namespace sled
#endif//  SLED_FUTURES_DETAIL_THEN_H
