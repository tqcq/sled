/**
 * @file     : random
 * @created  : Saturday Feb 03, 2024 11:47:02 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_RANDOM_H
#define SLED_RANDOM_H

#include <limits>
#include <stdint.h>

namespace sled {

class Random {
public:
    explicit Random(uint64_t seed);
    Random() = delete;
    Random(const Random &) = delete;
    Random &operator=(const Random &) = delete;

    template<typename T>
    T Rand() const
    {
        static_assert(
            std::numeric_limits<T>::is_integer && std::numeric_limits<T>::radix == 2
                && std::numeric_limits<T>::digits <= 32,
            "Rand is only supported for built-in integer types that are 32 bits or smaller");
        return static_cast<T>(NextOutput());
    }

    uint32_t Rand(uint32_t t) const;
    uint32_t Rand(uint32_t low, uint32_t high) const;
    int32_t Rand(int32_t low, int32_t high) const;
    double Gaussian(double mean, double standard_deviation) const;
    double Exponential(double lambda) const;

private:
    inline uint64_t NextOutput() const
    {
        state_ ^= state_ >> 12;
        state_ ^= state_ << 25;
        state_ ^= state_ >> 27;
        return state_ * 2685821657736338717ull;
    }

    mutable uint64_t state_;
};

template<>
float Random::Rand<float>() const;
template<>
double Random::Rand<double>() const;
template<>
bool Random::Rand<bool>() const;

}// namespace sled

#endif// SLED_RANDOM_H
