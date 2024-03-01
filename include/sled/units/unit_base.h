/**
 * @file     : unit_base
 * @created  : Friday Feb 02, 2024 17:46:50 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_UNITS_UNIT_BASE_H
#define SLED_UNITS_UNIT_BASE_H

#include "sled/numerics/divide_round.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <stdint.h>
#include <type_traits>

namespace sled {
namespace detail {

template<typename Unit_T>
class UnitBase {
public:
    UnitBase() = delete;

    static constexpr Unit_T Zero() { return Unit_T(0); }

    static constexpr Unit_T PlusInfinity() { return Unit_T(PlusInfinityVal()); }

    static constexpr Unit_T MinusInfinity() { return Unit_T(MinusInfinityVal()); }

    constexpr bool IsZero() const { return value_ == 0; }

    constexpr bool IsFinite() const { return !IsInfinite(); }

    constexpr bool IsInfinite() const
    {
        return value_ == PlusInfinityVal() || value_ == MinusInfinityVal();
    }

    constexpr bool IsPlusInfinity() const { return value_ == PlusInfinityVal(); }

    constexpr bool IsMinusInfinity() const { return value_ == MinusInfinityVal(); }

    constexpr bool operator==(const Unit_T &other) const { return value_ == other.value_; }

    constexpr bool operator!=(const Unit_T &other) const { return value_ != other.value_; }

    constexpr bool operator<(const Unit_T &other) const { return value_ < other.value_; }

    constexpr bool operator<=(const Unit_T &other) const { return value_ <= other.value_; }

    constexpr bool operator>(const Unit_T &other) const { return value_ > other.value_; }

    constexpr bool operator>=(const Unit_T &other) const { return value_ >= other.value_; }

    constexpr Unit_T RoundTo(const Unit_T &resolution) const
    {
        // ASSERT IsFinite();
        // ASSERT resolution.IsFinite();
        // ASSERT resolution.value_ > 0;
        return Unit_T((value_ + resolution.value_ / 2) / resolution.value_) * resolution.value_;
    }

    constexpr Unit_T RoundUpTo(const Unit_T &resolution) const
    {
        // ASSERT IsFinite();
        // ASSERT resolution.IsFinite();
        // ASSERT resolution.value_ > 0;
        return Unit_T((value_ + resolution.value_ - 1) / resolution.value_) * resolution.value_;
    }

    constexpr Unit_T RoundDownTo(const Unit_T &resolution) const
    {
        // ASSERT IsFinite();
        // ASSERT resolution.IsFinite();
        // ASSERT resolution.value_ > 0;
        return Unit_T(value_ / resolution.value_) * resolution.value_;
    }

protected:
    template<typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    static constexpr Unit_T FromValue(T value)
    {
        return Unit_T(static_cast<int64_t>(value));
    }

    template<typename T,
             typename std::enable_if<std::is_floating_point<T>::value>::type * = nullptr>
    static /*constexpr*/ Unit_T FromValue(T value)
    {
        if (value == std::numeric_limits<T>::infinity()) {
            return PlusInfinity();
        } else if (value == -std::numeric_limits<T>::infinity()) {
            return MinusInfinity();
        } else {
            return FromValue(static_cast<int64_t>(value));
        }
    }

    template<typename T, typename std::enable_if<std::is_integral<T>::value>::type * = nullptr>
    static constexpr Unit_T FromFraction(int64_t denominator, T value)
    {
        return Unit_T(static_cast<int64_t>(value * denominator));
    }

    template<typename T,
             typename std::enable_if<std::is_floating_point<T>::value>::type * = nullptr>
    static constexpr Unit_T FromFraction(int64_t denominator, T value)
    {
        return FromValue(value * denominator);
    }

    template<typename T = int64_t>
    constexpr typename std::enable_if<std::is_integral<T>::value, T>::type ToValue() const
    {
        return IsPlusInfinity() ? std::numeric_limits<T>::infinity()
            : IsMinusInfinity() ? -std::numeric_limits<T>::infinity()
                                : value_;
    }

    template<typename T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type ToValue() const
    {
        return IsPlusInfinity() ? std::numeric_limits<T>::infinity()
            : IsMinusInfinity() ? -std::numeric_limits<T>::infinity()
                                : value_;
    }

    template<typename T>
    constexpr T ToValueOr(T fallback_value) const
    {
        return IsFinite() ? value_ : fallback_value;
    }

    template<int64_t Denominator, typename T = int64_t>
    constexpr typename std::enable_if<std::is_integral<T>::value, T>::type ToFraction() const
    {
        return static_cast<T>(DivideRoundToNearst(value_, Denominator));
    }

    template<int64_t Denominator, typename T = int64_t>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type ToFraction() const
    {
        return ToValue<T>() * (1 / static_cast<T>(Denominator));
    }

    template<int64_t Denominator>
    constexpr int64_t ToFractionOr(int64_t fallback_value) const
    {
        return IsFinite() ? DivideRoundToNearst(value_, Denominator) : fallback_value;
    }

    template<int64_t Factor, typename T = int64_t>
    constexpr typename std::enable_if<std::is_integral<T>::value, T>::type ToMultiple() const
    {
        return static_cast<T>(ToValue() * Factor);
    }

    template<int64_t Factor, typename T>
    constexpr typename std::enable_if<std::is_floating_point<T>::value, T>::type ToMultiple() const
    {
        return ToValue() * Factor;
    }

    explicit constexpr UnitBase(int64_t value) : value_(value) {}

private:
    template<typename RelativeUnit_T>
    friend class RelativeUnit;

    static inline constexpr int64_t PlusInfinityVal()
    {
        return std::numeric_limits<int64_t>::max();
    }

    static inline constexpr int64_t MinusInfinityVal()
    {
        return std::numeric_limits<int64_t>::min();
    }

    Unit_T &AsSubClassRef() { return static_cast<Unit_T &>(*this); }

    constexpr const Unit_T &AsSubClassRef() const { return static_cast<const Unit_T &>(*this); }

    int64_t value_;
};

template<typename Unit_T>
class RelativeUnit : public UnitBase<Unit_T> {
public:
    constexpr Unit_T Clamped(Unit_T min_value, Unit_T max_value) const
    {
        return std::max(min_value, std::min(max_value, Unit_T()));
    }

    void Clamp(Unit_T min_value, Unit_T max_value) { *this = Clamped(min_value, max_value); }

    Unit_T operator+(const Unit_T other) const
    {
        if (this->IsPlusInfinity() || other.IsPlusInfinity()) {
            return this->PlusInfinity();
        } else if (this->IsMinusInfinity() || other->IsMinusInfinity()) {
            return this->MinusInfinity();
        }
        return UnitBase<Unit_T>::FromValue(this->ToValue() + other.ToValue());
    }

    Unit_T operator-(const Unit_T other) const
    {
        if (this->IsPlusInfinity() || other.IsMinusInfinity()) {
            return this->PlusInfinity();
        } else if (this->IsMinusInfinity() || other.IsPlusInfinity()) {
            return this->MinusInfinity();
        }
        return UnitBase<Unit_T>::FromValue(this->ToValue() - other.ToValue());
    }

    Unit_T &operator+=(const Unit_T other)
    {
        *this = *this + other;
        return this->AsSubClassRef();
    }

    Unit_T &operator-=(const Unit_T other)
    {
        *this = *this - other;
        return this->AsSubClassRef();
    }

    constexpr double operator/(const Unit_T other) const
    {
        return UnitBase<Unit_T>::template ToValue<double>() / other.template ToValue<double>();
    }

    template<typename T,
             typename std::enable_if<std::is_floating_point<T>::value, T>::type * = nullptr>
    constexpr Unit_T operator/(T scalar) const
    {
        return UnitBase<Unit_T>::FromValue(std::llround(this->ToValue() / scalar));
    }

    template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type * = nullptr>
    constexpr Unit_T operator/(T scalar) const
    {
        return UnitBase<Unit_T>::FromValue(this->ToValue() / scalar);
    }

    constexpr Unit_T operator*(double scalar) const
    {
        return UnitBase<Unit_T>::FromValue(std::llround(this->ToValue() * scalar));
    }

    constexpr Unit_T operator*(int64_t scalar) const
    {
        return UnitBase<Unit_T>::FromValue(this->ToValue() * scalar);
    }

    constexpr Unit_T operator*(int32_t scalar) const
    {
        return UnitBase<Unit_T>::FromValue(this->ToValue() * scalar);
    }

    constexpr Unit_T operator*(size_t scalar) const
    {
        return UnitBase<Unit_T>::FromValue(this->ToValue() * scalar);
    }

protected:
    // use UnitBase(value) as constructor
    using UnitBase<Unit_T>::UnitBase;
};

template<typename Unit_T>
inline constexpr Unit_T
operator*(double scalar, RelativeUnit<Unit_T> other)
{
    return other * scalar;
}

template<typename Unit_T>
inline constexpr Unit_T
operator*(int64_t scalar, RelativeUnit<Unit_T> other)
{
    return other * scalar;
}

template<typename Unit_T>
inline constexpr Unit_T
operator*(int32_t scalar, RelativeUnit<Unit_T> other)
{
    return other * scalar;
}

template<typename Unit_T>
inline constexpr Unit_T
operator*(size_t scalar, RelativeUnit<Unit_T> other)
{
    return other * scalar;
}

template<typename Unit_T>
inline Unit_T
operator-(RelativeUnit<Unit_T> other)
{
    if (other.IsPlusInfinity()) { return UnitBase<Unit_T>::MinusInfinity(); }
    if (other.IsMinusInfinity()) { return UnitBase<Unit_T>::PlusInfinity(); }
    return -1 * other;
}
}// namespace detail

}// namespace sled

#endif// SLED_UNITS_UNIT_BASE_H
