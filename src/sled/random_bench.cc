#include <sled/random.h>
#include <sled/testing/benchmark.h>

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { bool b = rand.Rand<bool>(); }
}).label("Random bool");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { int32_t i = rand.Rand<int8_t>(); }
}).label("Random int8_t");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { int32_t i = rand.Rand(-1000, 1000); }
}).label("Random int32_t");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { uint32_t i = rand.Rand<uint32_t>(); }
}).label("Random uint32_t");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { uint32_t i = rand.Rand(0u, 1000u); }
}).label("Random uint32_t range");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { double d = rand.Gaussian(0, 1); }
}).label("Gaussian(0, 1)");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { double d = rand.Exponential(1); }
}).label("Exponential(1)");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { float f = rand.Rand<float>(); }
}).label("Random float");

PICOBENCH([](picobench::state &s) {
    sled::Random rand(s.user_data());
    for (auto _ : s) { double d = rand.Rand<double>(); }
}).label("Random double");
