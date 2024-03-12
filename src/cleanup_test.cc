#include <gtest/gtest.h>
#include <sled/cleanup.h>
#include <sled/random.h>

TEST(Cleanup, TestCleanup)
{
    sled::Random rand(1314);
    for (int i = 0; i < 100; ++i) {
        int a = rand.Rand(10000);
        int b = rand.Rand(10000, 20000);
        {
            sled::Cleanup<> c([=, &a]() { a = b; });
        }
        ASSERT_EQ(a, b);
    }
}
