/**
 * @file     : divide_round
 * @created  : Saturday Feb 03, 2024 09:53:08 CST
 * @license  : MIT
 **/

#ifndef DIVIDE_ROUND_H
#define DIVIDE_ROUND_H

#include <type_traits>

namespace sled {
template<typename Dividend, typename Divisor>
inline auto
DivideRoundUp(Dividend dividend, Divisor divisor) -> decltype(Dividend() / Divisor())
{
    static_assert(std::is_integral<Dividend>(), "");
    static_assert(std::is_integral<Divisor>(), "");
    auto quotient = dividend / divisor;
    auto remainder = dividend % divisor;
    return quotient + (remainder > 0 ? 1 : 0);
}

template<typename Dividend, typename Divisor>
inline auto
DivideRoundToNearst(Dividend dividend, Divisor divisor) -> decltype(Dividend() / Divisor())
{
    static_assert(std::is_integral<Dividend>(), "");
    static_assert(std::is_integral<Divisor>(), "");
    if (dividend < Dividend(0)) {
        auto half_of_divisor = divisor / 2;
        auto quotient = dividend / divisor;
        auto remainder = dividend % divisor;
        if (half_of_divisor < -remainder) { --quotient; }
        return quotient;
    }
    auto half_of_divisor = (divisor - 1) / 2;
    auto quotient = dividend / divisor;
    auto remainder = dividend % divisor;
    if (half_of_divisor < remainder) { ++quotient; }
    return quotient;
}

}// namespace sled

#endif// DIVIDE_ROUND_H
