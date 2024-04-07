#ifndef SLED_EXP_DESIGN_PATTERNS_DISPATCHER_H
#define SLED_EXP_DESIGN_PATTERNS_DISPATCHER_H

#include "sled/synchronization/mutex.h"
#include <memory>
#include <set>

namespace sled {
namespace experimental {

template<typename T>
class Dispatcher {
public:
    enum class DispatchResult { kHandled, kNotFound };

    struct Handler {
        virtual ~Handler()                    = default;
        virtual bool HandleMessage(const T &) = 0;
        virtual bool OnMessage(T &&)          = 0;
    };

    virtual ~Dispatcher() = default;

    void AddHandler(std::shared_ptr<Handler> handler)
    {
        if (!handler) { return; }
        // sled::MutexLock lock(&mutex_);
        sled::SharedMutexWriteLock lock(&rwlock_);
        handlers_.insert(handler);
    }

    void RemoveHandler(std::shared_ptr<Handler> handler)
    {
        if (!handler) { return; }
        // sled::MutexLock lock(&mutex_);
        sled::SharedMutexWriteLock lock(&rwlock_);
        handlers_.erase(handler);
    }

    DispatchResult Dispatch(const T &message)
    {
        // sled::MutexLock lock(&mutex_);
        sled::SharedMutexReadLock lock(&rwlock_);
        for (auto &handler : handlers_) {
            if (handler->HandleMessage(message)) { return DispatchResult::kHandled; }
        }
        return DispatchResult::kNotFound;
    }

private:
    std::set<std::shared_ptr<Handler>> handlers_;
    sled::SharedMutex rwlock_;
    // sled::Mutex mutex_;
};

}// namespace experimental

}// namespace sled
#endif// SLED_EXP_DESIGN_PATTERNS_DISPATCHER_H
