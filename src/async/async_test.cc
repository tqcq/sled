#include <gtest/gtest.h>
#include <sled/async/async.h>
#include <sled/log/log.h>
#include <sled/system/fiber/wait_group.h>
#include <sled/system/thread.h>

TEST(Async, task)
{
    auto task1 = async::spawn([] { return 42; }).then([](int value) { return value * 3; }).then([](int value) {
        EXPECT_EQ(value, 126);
        return value;
    });
    task1.wait();
    EXPECT_EQ(126, task1.get());
}

TEST(Async, parallel_for)
{
    const int count = 1000;
    std::vector<int> values(count);
    async::parallel_for(async::irange(0, count), [&values](int x) {
        EXPECT_FALSE(values[x]);
        values[x] = true;
    });
    for (int i = 0; i < count; i++) { EXPECT_TRUE(values[i]) << i; }
}

TEST(Async, parallel_reduce)
{
    auto r = async::parallel_reduce(async::irange(1, 5), 0, [](int x, int y) {
        LOGD("", "{},{}", x, y);
        return x + y;
    });
    LOGD("", "{}", r);
}
