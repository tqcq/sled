#include <benchmark/benchmark.h>
#include <sled/random.h>
#include <sled/strings/base64.h>

static void
Base64Encode(benchmark::State &state)
{
    std::string input = "hello world\n";
    for (auto _ : state) { (void) sled::Base64::Encode(input); }
}

BENCHMARK(Base64Encode)->RangeMultiplier(10)->Range(10, 1000000);
