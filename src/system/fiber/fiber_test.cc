#include <gtest/gtest.h>
#include <sled/system/fiber/scheduler.h>
#include <sled/system/fiber/wait_group.h>

TEST(FiberScheduler, TestFiberScheduler)
{
    sled::Scheduler scheduler(sled::Scheduler::Config::allCores());
    scheduler.bind();
    defer(scheduler.unbind());

    std::atomic<int> counter = {0};
    sled::WaitGroup wg(1);
    sled::WaitGroup wg2(1000);
    for (int i = 0; i < 1000; i++) {
        sled::Schedule([&] {
            wg.Wait();
            wg2.Done();
            counter++;
        });
    }

    sled::Schedule([=] { wg.Done(); });
    wg2.Wait();
    ASSERT_EQ(counter.load(), 1000);
}
