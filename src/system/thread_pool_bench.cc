#include "sled/system/fiber/wait_group.h"
#include <benchmark/benchmark.h>
#include <future>
#include <sled/system/thread_pool.h>

static void
ThreadPoolBench(benchmark::State &state)
{
    sled::ThreadPool pool(-1);
    for (auto _ : state) {
        std::vector<std::future<int>> futures;
        for (int i = 0; i < state.range(0); i++) {
            std::future<int> f = pool.submit([]() { return 1; });
            futures.push_back(std::move(f));
        }
        for (auto &f : futures) { f.get(); }
    }
}

BENCHMARK(ThreadPoolBench)->RangeMultiplier(10)->Range(10, 10000);
