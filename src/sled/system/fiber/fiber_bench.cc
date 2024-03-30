#include <sled/system/fiber/scheduler.h>
#include <sled/system/fiber/wait_group.h>

static void
FiberSingleQueue(benchmark::State &state)
{
    for (auto _ : state) {
        state.PauseTiming();
        sled::Scheduler scheduler(sled::Scheduler::Config().setWorkerThreadCount(0));
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
        state.ResumeTiming();
        start_flag.Done();
        wg.Wait();
        state.PauseTiming();
    }
}

static void
FiberMultiQueue(benchmark::State &state)
{

    for (auto _ : state) {
        state.PauseTiming();
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
        state.ResumeTiming();
        start_flag.Done();
        wg.Wait();
        state.PauseTiming();
    }
}

BENCHMARK(FiberSingleQueue)->RangeMultiplier(10)->Range(10, 10000);
BENCHMARK(FiberMultiQueue)->RangeMultiplier(10)->Range(10, 10000);
