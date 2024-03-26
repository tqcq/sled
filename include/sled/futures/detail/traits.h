#ifndef SLED_FUTURES_DETAIL_TRAITS_H
#define SLED_FUTURES_DETAIL_TRAITS_H

#include "sled/exec/detail/invoke_result.h"

#include <memory>
#include <type_traits>

namespace sled {
namespace detail {
template<typename S, typename R>
struct ConnectResult {
    typedef decltype(std::declval<S>().Connect(std::declval<R>())) type;
};

template<typename S, typename R>
using ConnectResultT = typename ConnectResult<S, R>::type;

template<typename S, typename Lazy>
struct ContinueResult {
    typedef decltype(std::declval<Lazy>().Continue(std::declval<S>())) type;
};

template<typename S, typename Lazy>
using ContinueResultT = typename ContinueResult<S, Lazy>::type;

template<typename F, typename... Args>
using invoke_result_t = eggs::invoke_result_t<F, Args...>;

template<typename T>
using decay_t = typename std::decay<T>::type;

template<typename T>
struct move_on_copy {
    using type = typename std::remove_reference<T>::type;

    move_on_copy(type &&value) : value(std::move(value)) {}

    move_on_copy(const move_on_copy &other) : value(std::move(other.value)) {}

    move_on_copy(move_on_copy &&) = delete;
    move_on_copy &operator=(const move_on_copy &) = delete;

    mutable type value;
};

template<typename T>
move_on_copy<T>
make_move_on_copy(T &&value)
{
    return {std::move<T>(value)};
}

}// namespace detail
}// namespace sled
#endif// SLED_FUTURES_DETAIL_TRAITS_H
