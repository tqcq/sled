#include <benchmark/benchmark.h>
#include <sled/random.h>

class RandomFixture : public benchmark::Fixture {
    void SetUp(::benchmark::State &state) { rand_ = new sled::Random(1314); }

    void TearDown(::benchmark::State &state) { delete rand_; }

protected:
    sled::Random *rand_;
};

BENCHMARK_F(RandomFixture, bool)(benchmark::State &state)
{
    for (auto _ : state) { bool b = rand_->Rand<bool>(); }
}

BENCHMARK_F(RandomFixture, int32_t)(benchmark::State &state)
{
    for (auto _ : state) { int32_t i = rand_->Rand<int8_t>(); }
}

BENCHMARK_F(RandomFixture, int32_t_range)(benchmark::State &state)
{
    for (auto _ : state) { int32_t i = rand_->Rand(-1000, 1000); }
}

BENCHMARK_F(RandomFixture, uint32_t)(benchmark::State &state)
{
    for (auto _ : state) { uint32_t i = rand_->Rand<uint32_t>(); }
}

BENCHMARK_F(RandomFixture, uint32_t_range)(benchmark::State &state)
{
    for (auto _ : state) { uint32_t i = rand_->Rand(0u, 1000u); }
}

BENCHMARK_F(RandomFixture, Gaussian)(benchmark::State &state)
{
    for (auto _ : state) { double d = rand_->Gaussian(0, 1); }
}

BENCHMARK_F(RandomFixture, Exponential)(benchmark::State &state)
{
    for (auto _ : state) { double d = rand_->Exponential(1); }
}

BENCHMARK_F(RandomFixture, float)(benchmark::State &state)
{
    for (auto _ : state) { float f = rand_->Rand<float>(); }
}

BENCHMARK_F(RandomFixture, double)(benchmark::State &state)
{
    for (auto _ : state) { double d = rand_->Rand<double>(); }
}
