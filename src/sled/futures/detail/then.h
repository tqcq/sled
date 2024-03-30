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

    template<typename U, typename Ret = invoke_result_t<F, U>>
    void SetValue(U &&val)
    {
        if (stopped) { return; }
        try {
            receiver.SetValue(std::forward<Ret>(func(std::forward<U>(val))));
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
    using result_t  = invoke_result_t<F, typename decay_t<S>::result_t>;
    using this_type = ThenSender<S, F>;
    S sender;
    F func;

    template<typename R>
    ThenOperation<S, ThenReceiver<R, F>> Connect(R receiver)
    {
        return {sender.Connect(ThenReceiver<R, F>{receiver, func})};
    }

    template<typename Lazy>
    friend ContinueResultT<this_type, Lazy> operator|(this_type sender, Lazy lazy)
    {
        return lazy.Continue(sender);
    }
};

template<typename S, typename F>
ThenSender<S, F>
Then(S &&sender, F &&func)
{
    return {std::forward<S>(sender), std::forward<F>(func)};
}

template<typename F>
struct ThenLazy {
    F func;

    template<typename S>
    ThenSender<S, F> Continue(S sender) const
    {
        return {sender, func};
    }
};

template<typename F>
ThenLazy<F>
Then(F &&func)
{
    return {func};
}

}// namespace detail
}// namespace sled
#endif//  SLED_FUTURES_DETAIL_THEN_H
