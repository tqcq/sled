#include <benchmark/benchmark.h>
#include <sled/system/fiber/scheduler.h>
#include <sled/system/fiber/wait_group.h>

static void
MultiQueue(benchmark::State &state)
{
    sled::Scheduler scheduler(sled::Scheduler::Config::allCores());
    scheduler.bind();
    defer(scheduler.unbind());
    const int num_tasks = state.range(0);
    sled::WaitGroup wg(num_tasks);
    sled::WaitGroup start_flag(1);

    for (int i = 0; i < num_tasks; i++) {
        sled::Schedule([=] {
            start_flag.Wait();
            wg.Done();
        });
    }
    start_flag.Done();
    wg.Wait();
}

BENCHMARK(MultiQueue)->RangeMultiplier(10)->Range(10, 10000);
