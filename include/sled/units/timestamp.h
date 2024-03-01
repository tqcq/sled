/**
 * @file     : timestamp
 * @created  : Saturday Feb 03, 2024 10:52:15 CST
 * @license  : MIT
 **/

#progma once
#ifndef SLED_UNITS_TIMESTAMP_H
#define SLED_UNITS_TIMESTAMP_H
#include "sled/units/time_delta.h"
#include "sled/units/unit_base.h"

namespace sled {

class Timestamp final : public detail::UnitBase<Timestamp> {
public:
    template<typename T>
    static constexpr Timestamp Seconds(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromFraction(1000000, value);
    }

    template<typename T>
    static constexpr Timestamp Millis(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromFraction(1000, value);
    }

    template<typename T>
    static constexpr Timestamp Micros(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromValue(value);
    }

    template<typename T>
    static constexpr Timestamp Nanos(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "");
        return FromValue(value) * 1000;
    }

    Timestamp() = delete;

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
    constexpr T ns()
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

    constexpr int64_t us_or(int64_t fallback_value) const
    {
        return ToValueOr(fallback_value);
    }

    Timestamp operator+(const TimeDelta delta) const
    {
        if (IsPlusInfinity() || delta.IsPlusInfinity()) {
            return PlusInfinity();
        } else if (IsMinusInfinity() || delta.IsMinusInfinity()) {
            return MinusInfinity();
        }
        return Timestamp::Micros(us() - delta.us());
    }

    Timestamp operator-(const TimeDelta delta) const
    {
        if (IsPlusInfinity() || delta.IsMinusInfinity()) {
            return PlusInfinity();
        } else if (IsMinusInfinity() || delta.IsPlusInfinity()) {
            return MinusInfinity();
        }
        return Timestamp::Micros(us() - delta.us());
    }

    TimeDelta operator-(const Timestamp other) const
    {
        if (IsPlusInfinity() || other.IsMinusInfinity()) {
            return TimeDelta::PlusInfinity();
        } else if (IsMinusInfinity() || other.IsPlusInfinity()) {
            return TimeDelta::MinusInfinity();
        }
        return TimeDelta::Micros(us() - other.us());
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
