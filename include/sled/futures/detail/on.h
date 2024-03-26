#ifndef SLED_FUTURES_DETAIL_VIA_H
#define SLED_FUTURES_DETAIL_VIA_H
#include "traits.h"
#include <exception>
#include <functional>
#include <memory>

namespace sled {
namespace detail {
template<typename R, typename F>
struct OnReceiver {
    R receiver;
    F schedule;
    bool stopped = false;

    template<typename U>
    typename std::enable_if<std::is_rvalue_reference<U>::value
                            || (!std::is_copy_assignable<U>::value && !std::is_copy_constructible<U>::value)>::type
    SetValue(U &&val)
    {
        static_assert(std::is_rvalue_reference<decltype(val)>::value, "U must be an rvalue reference");
        if (stopped) { return; }
        try {
            auto moved = make_move_on_copy(val);
            schedule([this, moved]() mutable { receiver.SetValue(std::move(moved.value)); });
        } catch (...) {
            SetError(std::current_exception());
        }
    }

    template<typename U>
    typename std::enable_if<!std::is_rvalue_reference<U>::value
                            && (std::is_copy_assignable<U>::value || std::is_copy_constructible<U>::value)>::type
    SetValue(U &&val)
    {
        if (stopped) { return; }
        try {
            schedule([this, val] { receiver.SetValue(val); });
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
struct OnOperation {
    ConnectResultT<S, R> op;

    void Start() { op.Start(); }

    void Stop() { op.Stop(); }
};

template<typename S, typename F>
struct OnSender {
    using result_t  = typename S::result_t;
    using this_type = OnSender<S, F>;
    S sender;
    F schedule;

    template<typename R>
    OnOperation<S, OnReceiver<R, F>> Connect(R receiver)
    {
        return {sender.Connect(OnReceiver<R, F>{receiver, schedule})};
    }

    template<typename Lazy>
    friend ContinueResultT<this_type, Lazy> operator|(this_type sender, Lazy lazy)
    {
        return lazy.Continue(sender);
    }
};

template<typename F>
struct OnLazy {
    F func;

    template<typename S>
    OnSender<S, F> Continue(S sender) const
    {
        return {sender, func};
    }
};

template<typename S, typename F>
OnSender<S, F>
On(S sender, F &&schedule)
{
    return {sender, std::forward<F>(schedule)};
}

template<typename F>
OnLazy<F>
On(F &&schedule)
{
    return {schedule};
}

}// namespace detail
}// namespace sled
#endif// SLED_FUTURES_DETAIL_VIA_H
