/**
 * @file     : thread_local
 * @created  : Sunday Feb 04, 2024 18:33:04 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_SYNCHRONIZATION_THREAD_LOCAL_H
#define SLED_SYNCHRONIZATION_THREAD_LOCAL_H
#include <cstddef>
#include <thread>
#include <type_traits>

namespace sled {

using ThreadId = std::thread::id;
using ThreadLocalKey = uint64_t;

namespace detail {
class ThreadLocalManager final {
public:
    static ThreadLocalManager &Instance();
    ~ThreadLocalManager();
    static ThreadId CurrentThreadId();
    static ThreadLocalKey NextKey();
    void *Get(const ThreadId &thread_id, const ThreadLocalKey &key) const;
    void Delete(const ThreadId &thread_id, const ThreadLocalKey &key);
    void Delete(const ThreadLocalKey &key);
    void Set(const ThreadId &thread_id, const ThreadLocalKey &key, void *value);

private:
    ThreadLocalManager();
    class Impl;
    std::unique_ptr<Impl> impl_;
};
}// namespace detail

template<typename T>
class ThreadLocal final {
public:
    ThreadLocal() : key_(detail::ThreadLocalManager::NextKey()) {}

    ~ThreadLocal()
    { /*detail::ThreadLocalManager::Instance().Delete(key_); */
    }

    template<typename PointerT = T>
    typename std::enable_if<std::is_pointer<PointerT>::value, PointerT>::type
    Get() const
    {
        auto ptr = detail::ThreadLocalManager::Instance().Get(
            detail::ThreadLocalManager::CurrentThreadId(), key_);
        return static_cast<PointerT>(ptr);
    }

    template<typename IntT = T>
    typename std::enable_if<std::is_integral<IntT>::value, IntT>::type
    Get() const
    {
        static_assert(sizeof(IntT) <= sizeof(void *), "");
        auto ptr = detail::ThreadLocalManager::Instance().Get(
            detail::ThreadLocalManager::CurrentThreadId(), key_);
        return static_cast<IntT>(static_cast<char *>(ptr)
                                 - static_cast<char *>(0));
    }

    template<typename PointerT = T,
             typename std::enable_if<
                 std::is_pointer<PointerT>::value
                     || std::is_same<std::nullptr_t, PointerT>::value,
                 PointerT>::type = nullptr>
    void Set(PointerT value)
    {
        detail::ThreadLocalManager::Instance().Set(
            detail::ThreadLocalManager::CurrentThreadId(), key_,
            static_cast<void *>(value));
    }

    template<typename IntT = T,
             typename std::enable_if<std::is_integral<IntT>::value, IntT>::type
                 * = nullptr>
    void Set(IntT value)
    {
        static_assert(sizeof(IntT) <= sizeof(void *), "");
        detail::ThreadLocalManager::Instance().Set(
            detail::ThreadLocalManager::CurrentThreadId(), key_,
            (char *) 0 + value);
    }

private:
    const ThreadLocalKey key_;
};

}// namespace sled

#endif// SLED_SYNCHRONIZATION_THREAD_LOCAL_H
