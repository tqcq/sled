#ifndef SLED_FUTURES_DETAIL_VIA_H
#define SLED_FUTURES_DETAIL_VIA_H
#include "traits.h"
#include <exception>
#include <functional>
#include <memory>

namespace sled {
namespace detail {
template<typename R, typename F>
struct ViaReceiver {
    R receiver;
    F schedule;
    bool stopped = false;

    template<typename U>
    void SetValue(U &&val)
    {
        if (stopped) { return; }
        try {
            // auto func = std::bind(&R::SetValue, &receiver, std::forward<U>(val));
            // schedule(std::move(func));
            schedule([this, val]() mutable { receiver.SetValue(std::move(val)); });
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
struct ViaOperation {
    ConnectResultT<S, R> op;

    void Start() { op.Start(); }

    void Stop() { op.Stop(); }
};

template<typename S, typename F>
struct ViaSender {
    using result_t = typename S::result_t;
    S sender;
    F schedule;

    template<typename R>
    ViaOperation<S, ViaReceiver<R, F>> Connect(R receiver)
    {
        return {sender.Connect(ViaReceiver<R, F>{receiver, schedule})};
    }
};

template<typename S, typename F>
ViaSender<S, F>
Via(S sender, F &&schedule)
{
    return {sender, std::forward<F>(schedule)};
}

}// namespace detail
}// namespace sled
#endif// SLED_FUTURES_DETAIL_VIA_H
