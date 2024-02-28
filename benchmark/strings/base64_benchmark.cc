#include <benchmark/benchmark.h>
#include <sled/random.h>
#include <sled/strings/base64.h>
#include <sstream>

struct strings {};

static std::string
AllocRandomString(sled::Random &random, int len)
{
    std::stringstream ss;
    for (int i = len; i > 0; i--) { ss << random.Rand<char>(); }
    return ss.str();
}

void
BenchmarkBase64Encode(benchmark::State &state)
{
    state.PauseTiming();
    sled::Random random(2393);
    std::vector<std::string> test_data;
    for (int i = 0; i < state.range(0); i++) {
        test_data.emplace_back(AllocRandomString(random, state.range(1)));
    }

    state.ResumeTiming();
    for (int i = 0; i < state.range(2); i++) {
        for (const auto &str : test_data) {
            auto base64 = sled::Base64::Encode(str);
        }
    }
}

std::string
uint2str(unsigned int num)
{
    std::ostringstream oss;
    oss << num;
    return oss.str();
}

void
test(benchmark::State &state)
{
    for (int i = 0; i < 1000000; i++) (void) uint2str(i);
    state.end();
}

BENCHMARK(test);
/*
BENCHMARK(BenchmarkBase64Encode)
    ->ArgsProduct({
        // generate the num of strings
        benchmark::CreateRange(10, 1000, 10),
        // generate the length of each string
        benchmark::CreateRange(10, 1000, 10),
        benchmark::CreateRange(10, 1000, 10),
    });
*/
