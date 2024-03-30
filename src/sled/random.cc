#include "sled/random.h"
#include <cmath>

namespace sled {
Random::Random(uint64_t seed) : state_(seed) {}

uint32_t
Random::Rand(uint32_t t) const
{
    uint32_t x = NextOutput();
    uint64_t result = x * (static_cast<uint64_t>(t) + 1);
    result >>= 32;
    return result;
}

uint32_t
Random::Rand(uint32_t low, uint32_t high) const
{
    return Rand(high - low) + low;
}

int32_t
Random::Rand(int32_t low, int32_t high) const
{
    const int64_t low_i64{low};
    uint32_t range = static_cast<uint32_t>(high - low_i64);
    return static_cast<int32_t>(Rand(range) + low_i64);
}

template<>
float
Random::Rand<float>() const
{
    double result = NextOutput() - 1;
    result = result / static_cast<double>(0xFFFFFFFFFFFFFFFFull);
    return static_cast<float>(result);
}

template<>
double
Random::Rand<double>() const
{
    double result = NextOutput() - 1;
    result = result / static_cast<double>(0xFFFFFFFFFFFFFFFFull);
    return result;
}

template<>
bool
Random::Rand<bool>() const
{
    return Rand(0, 1) == 1;
}

double
Random::Gaussian(double mean, double standard_deviation) const
{
    const double kPi = 3.14159265358979323846;
    double u1 = static_cast<double>(NextOutput()) / static_cast<double>(0xFFFFFFFFFFFFFFFFull);
    double u2 = static_cast<double>(NextOutput()) / static_cast<double>(0xFFFFFFFFFFFFFFFFull);
    return mean + standard_deviation * sqrt(-2 * log(u1)) * cos(2 * kPi * u2);
}

double
Random::Exponential(double lambda) const
{
    double uniform = Rand<double>();
    return -log(uniform) / lambda;
}
}// namespace sled
