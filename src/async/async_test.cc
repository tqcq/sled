#include <gtest/gtest.h>
#include <sled/async/async.h>

TEST(Async, task)
{
    auto task1 = async::spawn([] { return 42; }).then([](int value) { return value * 3; }).then([](int value) {
        EXPECT_EQ(value, 126);
    });
    task1.wait();
}
