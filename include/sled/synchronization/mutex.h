/**
 * @file     : mutex
 * @created  : Saturday Feb 03, 2024 13:36:10 CST
 * @license  : MIT
 **/

#pragma once
#include "marl/conditionvariable.h"
#ifndef SLED_SYNCHRONIZATION_MUTEX_H
#define SLED_SYNCHRONIZATION_MUTEX_H

#include "sled/units/time_delta.h"
#include <chrono>
#include <condition_variable>
#include <marl/mutex.h>
#include <mutex>
#include <type_traits>

namespace sled {
namespace internal {
template<typename T>
struct HasLockAndUnlock {
    template<typename U,
             decltype(std::declval<U>().Lock()) * = nullptr,
             decltype(std::declval<U>().Unlock()) * = nullptr>
    static int Test(int);

    template<typename>
    static char Test(...);

    static constexpr bool value =
        std::is_same<decltype(Test<T>(0)), int>::value;
};
}// namespace internal

using Mutex = marl::mutex;

// class Mutex final {
// public:
//     Mutex() = default;
//     Mutex(const Mutex &) = delete;
//     Mutex &operator=(const Mutex &) = delete;
//
//     inline void Lock() { impl_.lock(); };
//
//     inline bool TryLock() { return impl_.try_lock(); }
//
//     inline void AssertHeld() {}
//
//     inline void Unlock() { impl_.unlock(); }
//
// private:
//     std::mutex impl_;
//     friend class ConditionVariable;
// };

class RecursiveMutex final {
public:
    RecursiveMutex() = default;
    RecursiveMutex(const RecursiveMutex &) = delete;
    RecursiveMutex &operator=(const RecursiveMutex &) = delete;

    inline void Lock() { impl_.lock(); }

    inline bool TryLock() { return impl_.try_lock(); }

    inline void AssertHeld() {}

    inline void Unlock() { impl_.unlock(); }

private:
    std::recursive_mutex impl_;
};

template<typename TLock,
         typename std::enable_if<internal::HasLockAndUnlock<TLock>::value,
                                 TLock>::type * = nullptr>
class LockGuard final {
public:
    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;

    explicit LockGuard(TLock *lock) : mutex_(lock) { mutex_->Lock(); };

    ~LockGuard() { mutex_->Unlock(); };

private:
    TLock *mutex_;
    friend class ConditionVariable;
};

class MutexGuard final {
public:
    MutexGuard(Mutex *mutex) : lock_(*mutex) {}

    MutexGuard(const MutexGuard &) = delete;
    MutexGuard &operator=(const MutexGuard &) = delete;

private:
    friend class ConditionVariable;
    marl::lock lock_;
};

using MutexLock = MutexGuard;
// using MutexGuard = marl::lock;
// using MutexLock = LockGuard<Mutex>;
// using MutexGuard = LockGuard<Mutex>;
using RecursiveMutexLock = LockGuard<RecursiveMutex>;

// class MutexLock final {
// public:
//     MutexLock(const MutexLock &) = delete;
//     MutexLock &operator=(const MutexLock &) = delete;
//
//     explicit MutexLock(Mutex *mutex) : mutex_(mutex) { mutex->Lock(); }
//
//     ~MutexLock() { mutex_->Unlock(); }
//
// private:
//     Mutex *mutex_;
// };
//
// class RecursiveMutexLock final {
// public:
//     RecursiveMutexLock(const RecursiveMutexLock &) = delete;
//     RecursiveMutexLock &operator=(const RecursiveMutexLock &) = delete;
//
//     explicit RecursiveMutexLock(RecursiveMutex *mutex) : mutex_(mutex)
//     {
//         mutex->Lock();
//     }
//
//     ~RecursiveMutexLock() { mutex_->Unlock(); }
//
// private:
//     RecursiveMutex *mutex_;
// };

class ConditionVariable final {
public:
    static constexpr TimeDelta kForever = TimeDelta::PlusInfinity();

    // inline ConditionVariable();

    inline void NotifyOne() { cv_.notify_one(); }

    inline void NotifyAll() { cv_.notify_all(); }

    template<typename Predicate>
    inline void Wait(MutexLock &lock, Predicate &&pred)
    {
        cv_.wait(lock, std::forward<Predicate>(pred));
    }

    template<typename Predicate>
    inline bool WaitFor(MutexLock &lock, TimeDelta timeout, Predicate &&pred)
    {
        if (timeout == TimeDelta::PlusInfinity()) {
            cv_.wait(lock.lock_, std::forward<Predicate>(pred));
            return true;
        } else {
            return cv_.wait_for(lock.lock_,
                                std::chrono::milliseconds(timeout.ms()),
                                std::forward<Predicate>(pred));
        }
    }

private:
    marl::ConditionVariable cv_;
};

// class ConditionVariable final {
// public:
//     static constexpr TimeDelta kForever = TimeDelta::PlusInfinity();
//     ConditionVariable() = default;
//     ConditionVariable(const ConditionVariable &) = delete;
//     ConditionVariable &operator=(const ConditionVariable &) = delete;
//
//     template<typename Predicate>
//     inline bool Wait(LockGuard<Mutex> &guard, Predicate pred)
//     {
//         std::unique_lock<std::mutex> lock(guard.mutex_->impl_, std::adopt_lock);
//         cv_.wait(lock, pred);
//         return true;
//     }
//
//     template<typename Predicate>
//     inline bool
//     WaitFor(LockGuard<Mutex> &guard, TimeDelta timeout, Predicate pred)
//     {
//         std::unique_lock<std::mutex> lock(guard.mutex_->impl_, std::adopt_lock);
//         if (timeout == kForever) {
//             cv_.wait(lock, pred);
//             return true;
//         } else {
//             return cv_.wait_for(lock, std::chrono::milliseconds(timeout.ms()),
//                                 pred);
//         }
//     }
//
//     // template<typename Predicate>
//     // bool WaitUntil(Mutex *mutex, TimeDelta timeout, Predicate pred)
//     // {}
//
//     inline void NotifyOne() { cv_.notify_one(); }
//
//     inline void NotifyAll() { cv_.notify_all(); }
//
// private:
//     std::condition_variable cv_;
// };

}// namespace sled

#endif// SLED_SYNCHRONIZATION_MUTEX_H
