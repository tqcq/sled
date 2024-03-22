#ifndef SLED_EXEC_DETAIL_RETRY_H
#define SLED_EXEC_DETAIL_RETRY_H
#include <atomic>
#include <memory>
#include <system_error>
#pragma once

#include "traits.h"

namespace sled {

struct RetryState {
    int retry_count;
    bool need_retry;
};

template<typename TReceiver>
struct RetryReceiver {
    TReceiver receiver;
    std::shared_ptr<RetryState> state;

    template<typename T>
    void SetValue(T &&value)
    {
        receiver.SetValue(value);
    }

    void SetError(std::exception_ptr err)
    {
        if (state->retry_count < 0) {}
    }

    void SetStopped() { receiver.SetStopped(); }
};

template<typename TSender, typename TReceiver>
struct RetryOperation {
    ConnectResultT<TSender, RetryReceiver<TReceiver>> op;
    std::shared_ptr<int> state;

    void Start() {}
};

template<typename TSender>
struct RetrySender {
    using S = typename std::remove_cv<typename std::remove_reference<TSender>::type>::type;
    using result_t = SenderResultT<S>;
    S sender;
    int retry_count;

    template<typename TReceiver>
    RetryOperation<TSender, TReceiver> Connect(TReceiver &&receiver)
    {
        auto retry_state = std::make_shared<RetryState>(new RetryState{retry_count, false});
        return {sender.Connect(RetryReceiver<TReceiver>{receiver, retry_state}), retry_state};
    }
};

template<typename TSender>
RetrySender<TSender>
Retry(TSender &&sender, int retry_count)
{
    return {std::forward<TSender>(sender), retry_count};
}

}// namespace sled
#endif//  SLED_EXEC_DETAIL_RETRY_H
