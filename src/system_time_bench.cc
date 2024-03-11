#include <benchmark/benchmark.h>
#include <sled/system_time.h>

static void
SystemTimeNanos(benchmark::State &state)
{
    for (auto _ : state) { (void) sled::SystemTimeNanos(); }
}

BENCHMARK(SystemTimeNanos);
