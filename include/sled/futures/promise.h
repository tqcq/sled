#pragma once
#ifndef SLED_FUTURES_PROMISE_H
#define SLED_FUTURES_PROMISE_H

#include <exception>
#include <type_traits>

namespace sled {
namespace {
enum class Progress {
    kNone,
    kTimeout,
    kDone,
    kRetrieved,
};

template<typename T>
struct State {
    Progress progress_ = Progress::kNone;
};

}// namespace

template<typename T>
class Future;
template<typename T>
class Try;

template<typename T>
class Promise {
public:
    static Promise<T> MakeEmpty() noexcept;
    Promise();
    ~Promise();
    Promise(Promise const &) = delete;
    Promise &operator=(Promise const &) = delete;
    Promise(Promise<T> &&other) noexcept;
    Promise &operator=(Promise<T> &&other) noexcept;

    Future<T> GetFuture();

    template<typename ValueType = T>
    typename std::enable_if<!std::is_void<ValueType>::value>::type
    SetValue(ValueType &&value)
    {}
};

}// namespace sled
#endif//  SLED_FUTURES_PROMISE_H
