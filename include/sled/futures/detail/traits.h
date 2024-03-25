#ifndef SLED_FUTURES_DETAIL_TRAITS_H
#define SLED_FUTURES_DETAIL_TRAITS_H

#include "sled/exec/detail/invoke_result.h"

#include <type_traits>

namespace sled {
namespace detail {
template<typename S, typename R>
struct ConnectResult {
    typedef decltype(std::declval<S>().Connect(std::declval<R>())) type;
};

template<typename S, typename R>
using ConnectResultT = typename ConnectResult<S, R>::type;

template<typename F, typename... Args>
using invoke_result_t = eggs::invoke_result_t<F, Args...>;

}// namespace detail
}// namespace sled
#endif// SLED_FUTURES_DETAIL_TRAITS_H
