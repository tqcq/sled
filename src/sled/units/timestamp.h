/**
 * @file     : timestamp
 * @created  : Saturday Feb 03, 2024 10:52:15 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_UNITS_TIMESTAMP_H
#define SLED_UNITS_TIMESTAMP_H
#include "sled/time_utils.h"
#include "sled/units/time_delta.h"
#include "sled/units/unit_base.h"

namespace sled {

class Timestamp final : public detail::UnitBase<Timestamp> {
public:
    static Timestamp Now() { return Timestamp::Nanos(TimeNanos()); }

    template<typename T>
    static constexpr Timestamp Seconds(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromFraction(1000000000, value);
    }

    template<typename T>
    static constexpr Timestamp Millis(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromFraction(1000000, value);
    }

    template<typename T>
    static constexpr Timestamp Micros(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromFraction(1000, value);
    }

    template<typename T>
    static constexpr Timestamp Nanos(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromValue(value);
    }

    Timestamp() = delete;

    bool IsExpired() const { return *this < Now(); }

    template<typename T = int64_t>
    constexpr T seconds() const
    {
        return ToFraction<1000000000, T>();
    }

    template<typename T = int64_t>
    constexpr T ms() const
    {
        return ToFraction<1000000, T>();
    }

    template<typename T = int64_t>
    constexpr T us() const
    {
        return ToFraction<1000, T>();
    }

    template<typename T = int64_t>
    constexpr T ns() const
    {
        return ToValue<T>();
    }

    constexpr int64_t seconds_or(int64_t fallback_value) const { return ToFractionOr<1000000000>(fallback_value); }

    constexpr int64_t ms_or(int64_t fallback_value) const { return ToFractionOr<1000000>(fallback_value); }

    constexpr int64_t us_or(int64_t fallback_value) const { return ToFractionOr<1000>(fallback_value); }

    constexpr int64_t ns_or(int64_t fallback_value) const { return ToValueOr(fallback_value); }

    Timestamp operator+(const TimeDelta delta) const
    {
        if (IsPlusInfinity() || delta.IsPlusInfinity()) {
            return PlusInfinity();
        } else if (IsMinusInfinity() || delta.IsMinusInfinity()) {
            return MinusInfinity();
        }
        return Timestamp::Nanos(ns() + delta.ns());
    }

    Timestamp operator-(const TimeDelta delta) const
    {
        if (IsPlusInfinity() || delta.IsMinusInfinity()) {
            return PlusInfinity();
        } else if (IsMinusInfinity() || delta.IsPlusInfinity()) {
            return MinusInfinity();
        }
        return Timestamp::Nanos(ns() - delta.ns());
    }

    TimeDelta operator-(const Timestamp other) const
    {
        if (IsPlusInfinity() || other.IsMinusInfinity()) {
            return TimeDelta::PlusInfinity();
        } else if (IsMinusInfinity() || other.IsPlusInfinity()) {
            return TimeDelta::MinusInfinity();
        }
        return TimeDelta::Nanos(ns() - other.ns());
    }

    Timestamp &operator-=(const TimeDelta delta)
    {
        *this = *this - delta;
        return *this;
    }

    Timestamp &operator+=(const TimeDelta delta)
    {
        *this = *this + delta;
        return *this;
    }

private:
    friend class detail::UnitBase<Timestamp>;
    using UnitBase::UnitBase;
    static constexpr bool one_sided = true;
};

std::string ToString(Timestamp value);

inline std::string
ToLogString(Timestamp value)
{
    return ToString(value);
}

inline std::ostream &
operator<<(std::ostream &stream, Timestamp value)
{
    stream << ToString(value);
    return stream;
}

}// namespace sled

#endif// SLED_UNITS_TIMESTAMP_H
