#include <gtest/gtest.h>
#include <sled/rx.h>

TEST(RX, interval)
{
    auto sc = sled::schedulers::make_current_thread();
    sled::observable<>::interval(std::chrono::seconds(1), sled::synchronize_in_one_worker(sc))
        .subscribe([](long counter) {});
}
