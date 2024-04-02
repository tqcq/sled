/**
 * @file     : mutex
 * @created  : Saturday Feb 03, 2024 13:36:10 CST
 * @license  : MIT
 **/

#ifndef SLED_SYNCHRONIZATION_MUTEX_H
#define SLED_SYNCHRONIZATION_MUTEX_H
#pragma once

#include "sled/lang/attributes.h"
#include "sled/units/time_delta.h"
#include <chrono>
#include <marl/conditionvariable.h>
#include <marl/mutex.h>
// #include <condition_variable>
// #include <mutex>
#include <type_traits>

namespace sled {
namespace internal {
template<typename T>
struct HasLockAndUnlock {
    template<typename U,
             decltype(std::declval<U>().Lock())   * = nullptr,
             decltype(std::declval<U>().Unlock()) * = nullptr>
    static int Test(int);

    template<typename>
    static char Test(...);

    static constexpr bool value = std::is_same<decltype(Test<T>(0)), int>::value;
};
}// namespace internal

// using Mutex = marl::mutex;

class SLED_LOCKABLE Mutex final {
public:
    Mutex()                         = default;
    Mutex(const Mutex &)            = delete;
    Mutex &operator=(const Mutex &) = delete;

    inline void Lock() SLED_EXCLUSIVE_LOCK_FUNCTION(impl_) { impl_.lock(); };

    inline bool TryLock() SLED_EXCLUSIVE_TRYLOCK_FUNCTION(true) { return impl_.try_lock(); }

    inline void AssertHeld() SLED_ASSERT_EXCLUSIVE_LOCK(impl_) {}

    inline void Unlock() SLED_UNLOCK_FUNCTION(impl_) { impl_.unlock(); }

private:
    marl::mutex impl_;
    friend class ConditionVariable;
    friend class MutexLock;
};

class SLED_LOCKABLE RecursiveMutex final {
public:
    RecursiveMutex()                                  = default;
    RecursiveMutex(const RecursiveMutex &)            = delete;
    RecursiveMutex &operator=(const RecursiveMutex &) = delete;

    inline void Lock() SLED_SHARED_LOCK_FUNCTION() { impl_.lock(); }

    inline bool TryLock() SLED_SHARED_TRYLOCK_FUNCTION(true) { return impl_.try_lock(); }

    inline void AssertHeld() {}

    inline void Unlock() SLED_UNLOCK_FUNCTION() { impl_.unlock(); }

private:
    std::recursive_mutex impl_;
};

class SLED_SCOPED_CAPABILITY RecursiveMutexLock final {
public:
    RecursiveMutexLock(const RecursiveMutexLock &)            = delete;
    RecursiveMutexLock &operator=(const RecursiveMutexLock &) = delete;

    explicit RecursiveMutexLock(RecursiveMutex *mutex) SLED_ACQUIRE_SHARED(mutex) : mutex_(mutex) { mutex->Lock(); }

    ~RecursiveMutexLock() SLED_RELEASE_SHARED(mutex_) { mutex_->Unlock(); }

private:
    RecursiveMutex *mutex_;
};

// public:
//     LockGuard(const LockGuard &) = delete;
//     LockGuard &operator=(const LockGuard &) = delete;
//
//     explicit LockGuard(TLock *lock) SLED_EXCLUSIVE_LOCK_FUNCTION(lock) : mutex_(lock) { mutex_->Lock(); };
//
//     ~LockGuard() SLED_UNLOCK_FUNCTION() { mutex_->Unlock(); };
//
// private:
//     TLock *mutex_;
//     friend class ConditionVariable;
// };
//
class SLED_SCOPED_CAPABILITY MutexLock final {
public:
    MutexLock(Mutex *mutex) SLED_ACQUIRE(mutex) : lock_(mutex->impl_) {}

    ~MutexLock() SLED_RELEASE() = default;

    MutexLock(const MutexLock &)            = delete;
    MutexLock &operator=(const MutexLock &) = delete;

private:
    friend class ConditionVariable;
    marl::lock lock_;
};

class ConditionVariable final {
public:
    static constexpr TimeDelta kForever = TimeDelta::PlusInfinity();

    // inline ConditionVariable();

    inline void NotifyOne() { cv_.notify_one(); }

    inline void NotifyAll() { cv_.notify_all(); }

    template<typename Predicate>
    inline void Wait(MutexLock &lock, Predicate &&pred)
    {
        cv_.wait(lock.lock_, std::forward<Predicate>(pred));
    }

    template<typename Predicate>
    inline bool WaitFor(MutexLock &lock, TimeDelta timeout, Predicate &&pred)
    {
        if (timeout.ns() < 0) { return pred(); }

        if (timeout == TimeDelta::PlusInfinity()) {
            cv_.wait(lock.lock_, std::forward<Predicate>(pred));
            return true;
        } else {
            return cv_.wait_for(lock.lock_, std::chrono::microseconds(timeout.us()), std::forward<Predicate>(pred));
        }
    }

private:
    marl::ConditionVariable cv_;
};

class SCOPED_CAPABILITY SharedMutex final {
public:
    enum class Mode {
        kReaderPriority,
        kWriterPriority,
    };

    inline SharedMutex(Mode mode = SharedMutex::Mode::kWriterPriority) : mode_(mode) {}

    inline void Lock() SLED_EXCLUSIVE_LOCK_FUNCTION()
    {
        wait_w_count_.fetch_add(1);

        sled::MutexLock lock(&mutex_);
        if (Mode::kReaderPriority == mode_) {
            // 读取优先，必须在没有任何读取的消费者的情况下才能持有锁
            cv_.Wait(lock, [this] { return r_count_ == 0 && w_count_ == 0 && wait_r_count_.load() == 0; });
            w_count_++;
        } else {
            // 写入优先，只要没有持有读锁的消费者，就可以加锁
            cv_.Wait(lock, [this] { return r_count_ == 0 && w_count_ == 0; });
            w_count_++;
            cv_.Wait(lock, [this] { return r_count_ == 0; });
        }
        wait_w_count_.fetch_sub(1);
    }

    inline void Unlock() SLED_UNLOCK_FUNCTION()
    {
        sled::MutexLock lock(&mutex_);
        w_count_--;
        if (w_count_ == 0) { cv_.NotifyAll(); }
    }

    inline void LockShared() SLED_SHARED_LOCK_FUNCTION()
    {
        wait_r_count_.fetch_add(1);
        sled::MutexLock lock(&mutex_);
        if (Mode::kReaderPriority == mode_) {
            cv_.Wait(lock, [this] { return w_count_ == 0; });
            r_count_++;
        } else {
            cv_.Wait(lock, [this] { return w_count_ == 0 && wait_w_count_.load() == 0; });
            r_count_++;
        }
        wait_r_count_.fetch_sub(1);
    }

    inline void UnlockShared() SLED_UNLOCK_FUNCTION()
    {
        sled::MutexLock lock(&mutex_);
        r_count_--;
        if (r_count_ == 0) { cv_.NotifyAll(); }
    }

private:
    const Mode mode_;
    sled::Mutex mutex_;
    sled::ConditionVariable cv_;
    int r_count_{0};
    int w_count_{0};
    std::atomic<int> wait_r_count_{0};
    std::atomic<int> wait_w_count_{0};
};

class SharedMutexReadLock final {
public:
    explicit SharedMutexReadLock(SharedMutex *mutex) : mutex_(mutex) { mutex_->LockShared(); }

    ~SharedMutexReadLock() { mutex_->UnlockShared(); }

private:
    SharedMutex *mutex_;
};

class SharedMutexWriteLock final {
public:
    explicit SharedMutexWriteLock(SharedMutex *mutex) : mutex_(mutex) { mutex_->Lock(); }

    ~SharedMutexWriteLock() { mutex_->Unlock(); }

private:
    SharedMutex *mutex_;
};

}// namespace sled

#endif// SLED_SYNCHRONIZATION_MUTEX_H
