#ifndef SLED_FUTURES_DETAIL_TRAITS_H
#define SLED_FUTURES_DETAIL_TRAITS_H

#include <type_traits>

namespace sled {
namespace detail {
template<typename S, typename R>
struct ConnectResult {
    typedef decltype(std::declval<S>().Connect(std::declval<R>())) type;
};

template<typename S, typename R>
using ConnectResultT = typename ConnectResult<S, R>::type;

}// namespace detail
}// namespace sled
#endif// SLED_FUTURES_DETAIL_TRAITS_H
