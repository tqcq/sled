#ifndef SLED_EXEC_DETAIL_SYNC_WAIT_H
#define SLED_EXEC_DETAIL_SYNC_WAIT_H
#pragma once

#include "sled/optional.h"
#include "sled/synchronization/mutex.h"
#include "traits.h"
#include <exception>

namespace sled {
struct SyncWaitData {
    sled::Mutex lock;
    sled::ConditionVariable cv;
    std::exception_ptr err;
    bool done = false;
};

template<typename T>
struct SyncWaitReceiver {
    SyncWaitData &data;
    sled::optional<T> &value;

    void SetValue(T &&val)
    {
        sled::MutexLock lock(&data.lock);
        value.emplace(val);
        data.done = true;
        data.cv.NotifyOne();
    }

    void SetError(std::exception_ptr err)
    {
        sled::MutexLock lock(&data.lock);
        data.err = err;
        data.done = true;
        data.cv.NotifyOne();
    }

    void SetStopped(std::exception_ptr err)
    {
        sled::MutexLock lock(&data.lock);
        data.done = true;
        data.cv.NotifyOne();
    }
};

template<typename TSender>
sled::optional<SenderResultT<TSender>>
SyncWait(TSender sender)
{
    using T = SenderResultT<TSender>;
    SyncWaitData data;
    sled::optional<T> value;

    auto op = sender.Connect(SyncWaitReceiver<T>{data, value});
    op.Start();

    sled::MutexLock lock(&data.lock);
    data.cv.Wait(lock, [&data] { return data.done; });

    if (data.err) { std::rethrow_exception(data.err); }

    return value;
}

}// namespace sled

#endif// SLED_EXEC_DETAIL_SYNC_WAIT_H
