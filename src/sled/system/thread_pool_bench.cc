#include <sled/system/fiber/wait_group.h>
#include <sled/system/thread_pool.h>
#include <sled/testing/benchmark.h>

static void
ThreadPoolBench(picobench::state &state)
{
    sled::ThreadPool pool(-1);
    for (auto _ : state) {
        std::future<int> f = pool.submit([]() { return 1; });
        (void) f.get();
    }
}

// BENCHMARK(ThreadPoolBench)->RangeMultiplier(10)->Range(10, 10000);
PICOBENCH(ThreadPoolBench);
