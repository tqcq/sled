#ifndef SLED_FUTURES_DETAIL_DELAY_H
#define SLED_FUTURES_DETAIL_DELAY_H

#include "sled/units/time_delta.h"
#include "traits.h"
#include <exception>

namespace sled {
namespace detail {

template<typename R>
struct DelayReceiver {
    R receiver;
    sled::TimeDelta delta;
    bool stopped = false;

    template<typename U>
    void SetValue(U &&val)
    {
        if (stopped) { return; }
        receiver.SetValue(std::forward<U>(val));
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
struct DelayOperation {
    ConnectResultT<S, R> op;

    void Start() { op.Start(); }

    void Stop() { op.Stop(); }
};

template<typename S>
struct DelaySender {
    using result_t = typename S::result_t;
    S sender;
    sled::TimeDelta delta;

    template<typename R>
    DelayOperation<S, DelayReceiver<R>> Connect(R receiver)
    {
        return {sender.Connect(DelayReceiver<R>{receiver, delta})};
    }
};

template<typename S>
DelaySender<S>
Delay(S sender, sled::TimeDelta const &delta)
{
    return {sender, delta};
}

}// namespace detail
}// namespace sled
#endif//  SLED_FUTURES_DETAIL_DELAY_H
