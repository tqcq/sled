/**
 * @file     : time_delta
 * @created  : Saturday Feb 03, 2024 10:31:29 CST
 * @license  : MIT
 **/
#pragma once

#ifndef SLED_UNITS_TIME_DELTA_H
#define SLED_UNITS_TIME_DELTA_H

#include "sled/units/unit_base.h"
#include <string>

namespace sled {

class TimeDelta : public detail::RelativeUnit<TimeDelta> {
public:
    template<typename T>
    static constexpr TimeDelta Minutes(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return Seconds(value * 60);
    }

    template<typename T>
    static constexpr TimeDelta Seconds(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromFraction(1000000, value);
    }

    template<typename T>
    static constexpr TimeDelta Millis(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromFraction(1000, value);
    }

    template<typename T>
    static constexpr TimeDelta Micros(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromValue(value);
    }

    TimeDelta() = delete;

    template<typename T = int64_t>
    constexpr T seconds() const
    {
        return ToFraction<1000000, T>();
    }

    template<typename T = int64_t>
    constexpr T ms() const
    {
        return ToFraction<1000, T>();
    }

    template<typename T = int64_t>
    constexpr T us() const
    {
        return ToValue<T>();
    }

    template<typename T = int64_t>
    constexpr T ns() const
    {
        return ToMultiple<1000, T>();
    }

    constexpr int64_t seconds_or(int64_t fallback_value) const
    {
        return ToFractionOr<1000000>(fallback_value);
    }

    constexpr int64_t ms_or(int64_t fallback_value) const
    {
        return ToFractionOr<1000>(fallback_value);
    }

    constexpr int64_t us_or(int64_t fallback_value) const { return ToValueOr(fallback_value); }

    constexpr TimeDelta Abs() const { return us() < 0 ? TimeDelta::Micros(-us()) : *this; }

private:
    friend class detail::UnitBase<TimeDelta>;
    using RelativeUnit::RelativeUnit;
    static constexpr bool one_sided = false;
};

std::string ToString(TimeDelta value);

inline std::string
ToLogString(TimeDelta value)
{
    return ToString(value);
}

inline std::ostream &
operator<<(std::ostream &stream, TimeDelta value)
{
    return stream << ToString(value);
}

}// namespace sled

#endif// SLED_UNITS_TIME_DELTA_H
