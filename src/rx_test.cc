#include <chrono>
#include <gtest/gtest.h>
#include <sled/rx.h>
#include <sled/time_utils.h>

TEST(RX, interval)
{
    // 2 + 4 + 6 = 12
    sled::observable<>::interval(std::chrono::milliseconds(300))
        .subscribe_on(rxcpp::synchronize_new_thread())
        .observe_on(rxcpp::observe_on_new_thread())
        .map([](long value) { return value * 2; })
        .take(3)
        .reduce(0, [](int acc, int value) { return acc + value; })
        .as_blocking()
        .subscribe([=](long counter) { EXPECT_EQ(counter, 12); }, []() {});
}
