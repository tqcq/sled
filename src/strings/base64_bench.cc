#include <benchmark/benchmark.h>
#include <sled/random.h>
#include <sled/strings/base64.h>
#include <sstream>

static std::string
RandomString(size_t length)
{
    static const char chars[] = "0123456789"
                                "abcdefghijklmnopqrstuvwxyz"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::stringstream ss;
    sled::Random rand(1314);
    while (length--) { ss << chars[rand.Rand(sizeof(chars))]; }
    return ss.str();
}

static void
Base64Encode(benchmark::State &state)
{
    for (auto _ : state) {
        state.PauseTiming();
        std::string input = RandomString(state.range(0));
        state.ResumeTiming();
        (void) sled::Base64::Encode(input);
    }
}

static void
Base64Decode(benchmark::State &state)
{
    for (auto _ : state) {
        state.PauseTiming();
        std::string input = RandomString(state.range(0));
        std::string base64_input = sled::Base64::Encode(input);
        state.ResumeTiming();
        (void) sled::Base64::Decode(base64_input);
    }
}

BENCHMARK(Base64Encode)->RangeMultiplier(10)->Range(10, 1000000);
BENCHMARK(Base64Decode)->RangeMultiplier(10)->Range(10, 1000000);
