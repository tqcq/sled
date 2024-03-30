#pragma once
#ifndef SLED_APPLY_H
#define SLED_APPLY_H
#include <functional>
#include <tuple>

namespace sled {
namespace detail {
template<int... Seq>
struct Sequence {};

template<int N, int... Seq>
struct MakeSeq : MakeSeq<N - 1, N - 1, Seq...> {};

template<int... Seq>
struct MakeSeq<0, Seq...> {
    using type = Sequence<Seq...>;
};

template<typename ReturnT, typename Func, typename Tuple, int... Seq>
ReturnT
ApplyImpl(const Func &func, const Tuple &tuple, const Sequence<Seq...> &)
{
    return std::bind(func, std::get<Seq>(tuple)...)();
}

struct VoidTag {};

}// namespace detail

template<typename ReturnT,
         typename Func,
         typename Tuple,
         typename std::enable_if<!std::is_void<ReturnT>::value, ReturnT>::type
             * = nullptr>
ReturnT
apply(const Func &func, const Tuple &tuple)
{
    return detail::ApplyImpl<ReturnT>(
        func, tuple,
        typename detail::MakeSeq<std::tuple_size<Tuple>::value>::type());
}

template<typename ReturnT = void,
         typename Func,
         typename Tuple,
         typename std::enable_if<std::is_void<ReturnT>::value,
                                 detail::VoidTag>::type * = nullptr>
void
apply(const Func &func, const Tuple &tuple)
{
    detail::ApplyImpl<ReturnT>(
        func, tuple,
        typename detail::MakeSeq<std::tuple_size<Tuple>::value>::type());
}

}// namespace sled
#endif// SLED_APPLY_H
