/*
The MIT License (MIT)

Copyright (c) 2015 Julius Ikkala

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef FIXED_POINT_H_
#define FIXED_POINT_H_
#pragma once
#include <Eigen/Eigen>
#include <cmath>
#include <cstdint>
#include <fmt/ostream.h>
#include <type_traits>

//==========================================================================
//Fixed-point numbers
//==========================================================================
namespace fp {
/*The fixed-point number class. f is the amount of fractional bits, I is
          the internal type. If I is signed, q will be signed.*/
template<unsigned f, typename I>
struct q {
    I i;

    q(int i = 0); /*Integer literals are int*/
    q(unsigned u);
    q(uintmax_t s);
    q(double d);
    q(long double d);
    template<unsigned fb>
    q(q<fb, I> a);

    operator int() const;
    operator float() const;
    operator double() const;
    operator long double() const;
    template<unsigned fb>
    q<f, I> operator+(q<fb, I> b) const;
    template<unsigned fb>
    q<f, I> operator-(q<fb, I> b) const;
    template<unsigned fb>
    q<f, I> operator*(q<fb, I> b) const;
    template<unsigned fb>
    q<f, I> operator/(q<fb, I> b) const;
    template<unsigned fb>
    q<f, I> operator%(q<fb, I> b) const;

    template<typename T>
    q<f, I> operator+(T b) const;
    template<typename T>
    q<f, I> operator-(T b) const;
    template<typename T>
    q<f, I> operator*(T b) const;
    template<typename T>
    q<f, I> operator/(T b) const;
    template<typename T>
    q<f, I> operator%(T b) const;

    template<typename T>
    q<f, I> &operator+=(T b);
    template<typename T>
    q<f, I> &operator-=(T b);
    template<typename T>
    q<f, I> &operator*=(T b);
    template<typename T>
    q<f, I> &operator/=(T b);
    template<typename T>
    q<f, I> &operator%=(T b);

    q<f, I> operator-() const;

    template<unsigned fb>
    q<f, I> &operator=(q<fb, I> b);
    q<f, I> &operator=(int b);
    q<f, I> &operator=(double b);
    q<f, I> &operator=(long double b);

    template<unsigned fb>
    bool operator>=(q<fb, I> b) const;
    template<unsigned fb>
    bool operator>(q<fb, I> b) const;
    template<unsigned fb>
    bool operator<=(q<fb, I> b) const;
    template<unsigned fb>
    bool operator<(q<fb, I> b) const;
    template<unsigned fb>
    bool operator==(q<fb, I> b) const;
    template<unsigned fb>
    bool operator!=(q<fb, I> b) const;

    friend std::ostream &operator<<(std::ostream &os, const q<f, I> &fp)
    {
        os << (float) fp;
        return os;
    }

    friend std::istream &operator>>(std::istream &is, q<f, I> &fp)
    {
        float val;
        is >> val;
        fp = val;
        return is;
    }
};

typedef q<16, uint32_t> UQ16_16;
typedef q<32, uint64_t> UQ32_32;
typedef q<16, uint64_t> UQ48_16;
typedef q<16, int32_t> Q16_16;
typedef q<32, int64_t> Q32_32;
typedef q<16, int64_t> Q48_16;

/*Returns the absolute value of x*/
template<unsigned f, typename I>
q<f, I> abs(q<f, I> x);
}// namespace fp

//==========================================================================
//Internal helper functions and classes
//==========================================================================
namespace fp_internal {
/*Count leading zeroes*/
template<typename T>
unsigned
clz(T x)
{
    unsigned i = 0;
    while (!(x & (((T) 1) << (sizeof(T) * 8 - 1))) && (x <<= 1)) ++i;
    return i;
}

/*Used to select the correct shift operator in compile-time*/
template<int shift, bool rsh, bool zero, typename T>
struct shifter {
    static T op(T x);
};

template<int shift, typename T>
struct shifter<shift, true, false, T> {
    static T op(T x) { return x >> shift; };
};

template<int shift, typename T>
struct shifter<shift, false, false, T> {
    static T op(T x) { return x << -shift; };
};

template<int shift, bool rsh, typename T>
struct shifter<shift, rsh, true, T> {
    static T op(T x) { return 0; };
};

template<typename T>
constexpr T
abs_helper(const T x) noexcept
{
    return (// deal with signed-zeros
        x == T(0) ? T(0) :
                  // else
            x < T(0) ? -x
                     : x);
}

/*Signed right shift. Accepts negative values of shift and never
          complains about too big shifts. Compile-time version.*/
template<int shift, typename T>
constexpr T
signed_rsh(T x)
{
    return shifter<shift, (shift >= 0), (abs_helper(shift) >= sizeof(T) * 8), T>::op(x);
}

/*Signed right shift, run time version.*/
template<typename T>
T
signed_rsh(T x, int shift)
{
    return std::abs(shift) < sizeof(T) * 8 ? (shift < 0 ? x << -shift : x >> shift) : 0;
}

/*Signed left shift, compile-time version*/
template<int shift, typename T>
T
signed_lsh(T x)
{
    return signed_rsh<-shift, T>(x);
}

/*Signed left shift, run time version*/
template<typename T>
T
signed_lsh(T x, int shift)
{
    return std::abs(shift) < sizeof(T) * 8 ? (shift < 0 ? x >> -shift : x << shift) : 0;
}

/*Multiplies and simultaneously right-shifts the argument values,
          without losing the high bits. Compile-time version*/
template<int shift, typename T>
T
mul_rsh(T a, T b)
{
    typedef typename std::make_unsigned<T>::type U;
    static const int bits = sizeof(T) * 8;
    static const int halfbits = sizeof(T) * 4;
    static const U lowmask = (((T) 1) << (halfbits)) - 1;
    static const U highmask = lowmask << (halfbits);
    T a1 = a >> (halfbits);
    T a2 = a - (a & highmask);
    T b1 = b >> (halfbits);
    T b2 = b - (b & highmask);
    T a1b1 = a1 * b1;
    T a1b2 = a1 * b2;
    T a2b1 = a2 * b1;
    T a2b2 = a2 * b2;
    U high = a1b1 + (a2b1 >> (halfbits)) + (a1b2 >> (halfbits));

    U low = a2b2;
    high += (low += (a2b1 << halfbits)) < a2b2;
    U tmp = low;
    high += (low += (a1b2 << halfbits)) < tmp;
    low = signed_rsh<shift>(low);
    low += signed_lsh<bits - shift>(high);
    return low;
}

/*Run time version*/
template<typename T>
T
mul_rsh(T a, T b, int shift)
{
    typedef typename std::make_unsigned<T>::type U;
    static const int bits = sizeof(T) * 8;
    static const int halfbits = sizeof(T) * 4;
    static const U lowmask = (((T) 1) << (halfbits)) - 1;
    static const U highmask = lowmask << (halfbits);
    T a1 = a >> (halfbits);
    T a2 = a - (a & highmask);
    T b1 = b >> (halfbits);
    T b2 = b - (b & highmask);
    T a1b1 = a1 * b1;
    T a1b2 = a1 * b2;
    T a2b1 = a2 * b1;
    T a2b2 = a2 * b2;
    U high = a1b1 + (a2b1 >> (halfbits)) + (a1b2 >> (halfbits));

    U low = a2b2;
    high += (low += (a2b1 << halfbits)) < a2b2;
    U tmp = low;
    high += (low += (a1b2 << halfbits)) < tmp;
    low = signed_rsh(low, shift);
    low += signed_lsh(high, bits - shift);
    return low;
}
}// namespace fp_internal

//==========================================================================
//Implementation
//==========================================================================
template<unsigned f, typename I>
fp::q<f, I>::q(int u)
{
    i = fp_internal::signed_lsh<f>((I) u);
}

template<unsigned f, typename I>
fp::q<f, I>::q(unsigned u)
{
    i = fp_internal::signed_lsh<f>((I) u);
}

template<unsigned f, typename I>
fp::q<f, I>::q(uintmax_t s)
{
    i = fp_internal::signed_lsh<f>((I) s);
}

template<unsigned f, typename I>
fp::q<f, I>::q(double d)
{
    int exp = 0;
    double tmp = frexp(d, &exp);
    i = lroundl(ldexp(tmp, exp + f));
}

template<unsigned f, typename I>
fp::q<f, I>::q(long double d)
{
    int exp = 0;
    long double tmp = frexp(d, &exp);
    i = llroundl(ldexp(tmp, exp + f));
}

template<unsigned f, typename I>
template<unsigned fb>
fp::q<f, I>::q(q<fb, I> a)
{
    i = fp_internal::signed_rsh<fb - f>(a.i);
}

template<unsigned f, typename I>
fp::q<f, I>::operator int() const
{
    return fp_internal::signed_rsh<f>(i);
}

template<unsigned f, typename I>
fp::q<f, I>::operator float() const
{
    return ldexpf((float) i, -(int) f);
}

template<unsigned f, typename I>
fp::q<f, I>::operator double() const
{
    return ldexp((double) i, -(int) f);
}

template<unsigned f, typename I>
fp::q<f, I>::operator long double() const
{
    return ldexp((long double) i, -(int) f);
}

template<unsigned f, typename I>
template<unsigned fb>
fp::q<f, I>
fp::q<f, I>::operator+(q<fb, I> b) const
{
    q<f, I> t;
    t.i = i + fp_internal::signed_rsh<fb - f>(b.i);
    return t;
}

template<unsigned f, typename I>
template<unsigned fb>
fp::q<f, I>
fp::q<f, I>::operator-(q<fb, I> b) const
{
    q<f, I> t;
    t.i = i - fp_internal::signed_rsh<fb - f>(b.i);
    return t;
}

template<unsigned f, typename I>
template<unsigned fb>
fp::q<f, I>
fp::q<f, I>::operator*(q<fb, I> b) const
{
    q<f, I> t;
    t.i = fp_internal::mul_rsh<fb>(i, b.i);
    return t;
}

template<unsigned f, typename I>
template<unsigned fb>
fp::q<f, I>
fp::q<f, I>::operator/(q<fb, I> b) const
{
    static const I msb = (I) 1 << (sizeof(I) * 8 - 1);//Most significant bit for the type
    //Make b positive so that leading zeroes can be properly computed
    I abs_b = b.i < 0 ? -b.i : b.i;
    unsigned lz = fp_internal::clz(abs_b);//Amount of leading zeroes
    //normalize b to [0.5, 1.0[, where all digits are after radix
    I d = (abs_b << lz);
    q<sizeof(I) * 8 + 1, typename std::make_unsigned<I>::type> e;
    e.i = (~d + 1) << 1;//[0, 0.5[
    //r is the reciprocal of d
    q<sizeof(I) * 8 - 1, typename std::make_unsigned<I>::type> r(1);
    for (unsigned i = 0; i < sizeof(I) - 1; ++i) {
        r = r + r * e;
        e = e * e;
    }
    q<f, I> t;
    t.i = (I) fp_internal::mul_rsh(//adjust the radix point of (this*r)
        r.i, (typename std::make_unsigned<I>::type)(this->i < 0 ? -this->i : this->i),
        sizeof(i) * 16 - fb - lz - (d == msb) - 1);
    t.i = (b.i < 0) ^ (this->i < 0) ? -t.i : t.i;//set correct sign
    return t;
}

template<unsigned f, typename I>
template<unsigned fb>
fp::q<f, I>
fp::q<f, I>::operator%(q<fb, I> b) const
{
    q<f, I> t;
    t.i = i % fp_internal::signed_rsh<fb - f>(b.i);
    return t;
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I>
fp::q<f, I>::operator+(T b) const
{
    return *this + q<f, I>(b);
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I>
fp::q<f, I>::operator-(T b) const
{
    return *this - q<f, I>(b);
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I>
fp::q<f, I>::operator*(T b) const
{
    return *this * q<f, I>(b);
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I>
fp::q<f, I>::operator/(T b) const
{
    return *this / q<f, I>(b);
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I>
fp::q<f, I>::operator%(T b) const
{
    return *this % q<f, I>(b);
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I> &
fp::q<f, I>::operator+=(T b)
{
    return *this = *this + b;
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I> &
fp::q<f, I>::operator-=(T b)
{
    return *this = *this - b;
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I> &
fp::q<f, I>::operator*=(T b)
{
    return *this = *this * b;
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I> &
fp::q<f, I>::operator/=(T b)
{
    return *this = *this / b;
}

template<unsigned f, typename I>
template<typename T>
fp::q<f, I> &
fp::q<f, I>::operator%=(T b)
{
    return *this = *this % b;
}

template<unsigned f, typename I>
fp::q<f, I>
fp::q<f, I>::operator-() const
{
    q<f, I> t;
    t.i = -i;
    return t;
}

template<unsigned f, typename I>
template<unsigned fb>
fp::q<f, I> &
fp::q<f, I>::operator=(q<fb, I> b)
{
    i = fp_internal::signed_rsh<fb - f>(b.i);
    return *this;
}

template<unsigned f, typename I>
fp::q<f, I> &
fp::q<f, I>::operator=(int b)
{
    i = fp_internal::signed_lsh<f>((I) b);
    return *this;
}

template<unsigned f, typename I>
fp::q<f, I> &
fp::q<f, I>::operator=(double b)
{
    int exp = 0;
    double tmp = frexp(b, &exp);
    i = llroundl(ldexp(tmp, exp + f));
    return *this;
}

template<unsigned f, typename I>
fp::q<f, I> &
fp::q<f, I>::operator=(long double b)
{
    int exp = 0;
    long double tmp = frexp(b, &exp);
    i = llroundl(ldexp(tmp, exp + f));
    return *this;
}

template<unsigned f, typename I>
template<unsigned fb>
bool
fp::q<f, I>::operator>=(q<fb, I> b) const
{
    static const unsigned bits = sizeof(I) * 8;
    I ma = fp_internal::signed_rsh<f>(i);
    I mb = fp_internal::signed_rsh<fb>(b.i);
    return (ma > mb)
        || ((ma == mb)
            && ((typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - f>(i)
                >= (typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - fb>(b.i)));
}

template<unsigned f, typename I>
template<unsigned fb>
bool
fp::q<f, I>::operator>(q<fb, I> b) const
{
    static const unsigned bits = sizeof(I) * 8;
    I ma = fp_internal::signed_rsh<f>(i);
    I mb = fp_internal::signed_rsh<fb>(b.i);
    return (ma > mb)
        || ((ma == mb)
            && ((typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - f>(i)
                > (typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - fb>(b.i)));
}

template<unsigned f, typename I>
template<unsigned fb>
bool
fp::q<f, I>::operator<=(q<fb, I> b) const
{
    static const unsigned bits = sizeof(I) * 8;
    I ma = fp_internal::signed_rsh<f>(i);
    I mb = fp_internal::signed_rsh<fb>(b.i);
    return (ma < mb)
        || ((ma == mb)
            && ((typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - f>(i)
                <= (typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - fb>(b.i)));
}

template<unsigned f, typename I>
template<unsigned fb>
bool
fp::q<f, I>::operator<(q<fb, I> b) const
{
    static const unsigned bits = sizeof(I) * 8;
    I ma = fp_internal::signed_rsh<f>(i);
    I mb = fp_internal::signed_rsh<fb>(b.i);
    return (ma < mb)
        || ((ma == mb)
            && ((typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - f>(i)
                < (typename std::make_unsigned<I>::type) fp_internal::signed_lsh<bits - fb>(b.i)));
}

template<unsigned f, typename I>
template<unsigned fb>
bool
fp::q<f, I>::operator==(q<fb, I> b) const
{
    return fp_internal::signed_rsh<f - fb>(i) == b.i && fp_internal::signed_rsh<fb - f>(b.i) == i;
}

template<unsigned f, typename I>
template<unsigned fb>
bool
fp::q<f, I>::operator!=(q<fb, I> b) const
{
    return fp_internal::signed_rsh<f - fb>(i) != b.i || fp_internal::signed_rsh<fb - f>(b.i) != i;
}

template<unsigned f, typename I>
fp::q<f, I>
fp::abs(q<f, I> x)
{
    return (x.i >= 0 ? x : -x);
}

namespace sled {
template<unsigned bits, typename T>
using FixedPoint = fp::q<bits, T>;

using FixedFloat = fp::Q32_32;

template<unsigned bits, typename T>
inline const FixedPoint<bits, T>
conj(const FixedPoint<bits, T> &val)
{
    return val;
}

template<unsigned bits, typename T>
inline const FixedPoint<bits, T>
real(const FixedPoint<bits, T> &val)
{
    return val;
}

template<unsigned bits, typename T>
inline const FixedPoint<bits, T>
imag(const FixedPoint<bits, T> &val)
{
    return 0.;
}

template<unsigned bits, typename T>
inline const FixedPoint<bits, T>
abs(const FixedPoint<bits, T> &val)
{
    return fp::abs(val);
}

template<unsigned bits, typename T>
inline const FixedPoint<bits, T>
abs2(const FixedPoint<bits, T> &val)
{
    return val * val;
}

}// namespace sled

template<unsigned bits, typename T>
struct fmt::formatter<sled::FixedPoint<bits, T>> : ostream_formatter {};

namespace Eigen {
template<unsigned bits, typename T>
struct NumTraits<sled::FixedPoint<bits, T>> : NumTraits<float> {
    typedef sled::FixedPoint<bits, T> Real;
    typedef sled::FixedPoint<bits, T> NonInteger;
    typedef sled::FixedPoint<bits, T> Nested;

    enum {
        IsComplex = 0,
        IsInteger = 0,
        IsSigned = std::is_signed<T>::value,
        RequireInitialization = 1,
        ReadCost = 1,
        AddCost = 3,
        MulCost = 3
    };
};

template<typename BinaryOp, unsigned bits, typename T>
struct ScalarBinaryOpTraits<sled::FixedPoint<bits, T>, float, BinaryOp> {
    typedef sled::FixedPoint<bits, T> ReturnType;
};

template<typename BinaryOp, unsigned bits, typename T>
struct ScalarBinaryOpTraits<float, sled::FixedPoint<bits, T>, BinaryOp> {
    typedef sled::FixedPoint<bits, T> ReturnType;
};

}// namespace Eigen

#endif
