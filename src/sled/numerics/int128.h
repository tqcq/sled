#ifndef SLED_NUMERICS_INT128_H
#define SLED_NUMERICS_INT128_H
#include <string>

#pragma once

namespace sled {
class Int128 {
    constexpr Int128(int v);
    constexpr Int128(unsigned int v);
    constexpr Int128(long v);
    constexpr Int128(unsigned long v);
    constexpr Int128(long long v);
    constexpr Int128(unsigned long long v);
    constexpr Int128(const Int128 &v);

    Int128 &operator=(int v);
    Int128 &operator=(unsigned int v);
    Int128 &operator=(long v);
    Int128 &operator=(unsigned long v);
    Int128 &operator=(long long v);
    Int128 &operator=(unsigned long long v);
    Int128 &operator=(const Int128 &v);

    Int128 &operator+=(const Int128 &v);
    Int128 &operator-=(const Int128 &v);
    Int128 &operator*=(const Int128 &v);
    Int128 &operator/=(const Int128 &v);
    Int128 &operator%=(const Int128 &v);
    Int128 &operator++(int);
    Int128 &operator--(int);
    Int128 &operator&=(const Int128 &v);
    Int128 &operator|=(const Int128 &v);
    Int128 &operator^=(const Int128 &v);
    Int128 &operator<<=(int v);
    Int128 &operator>>=(int v);

    constexpr explicit operator bool() const;
    constexpr explicit operator char() const;
    constexpr explicit operator unsigned char() const;
    constexpr explicit operator short() const;
    constexpr explicit operator unsigned short() const;
    constexpr explicit operator int() const;
    constexpr explicit operator unsigned int() const;
    constexpr explicit operator long() const;
    constexpr explicit operator unsigned long() const;
    constexpr explicit operator long long() const;
    constexpr explicit operator unsigned long long() const;
    explicit operator float() const;
    explicit operator double() const;
    explicit operator long double() const;

private:
    std::string ToString() const;

    uint64_t lo_;
    uint64_t hi_;
};
}// namespace sled

#endif// SLED_NUMERICS_INT128_H
