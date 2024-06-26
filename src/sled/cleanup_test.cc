#include <sled/cleanup.h>
#include <sled/random.h>

TEST_SUITE("Cleanup")
{
    TEST_CASE("assign")
    {
        sled::Random rand(1314);
        for (int i = 0; i < 100; ++i) {
            int a = rand.Rand(10000);
            int b = rand.Rand(10000, 20000);
            {
                sled::Cleanup<> c([=, &a]() { a = b; });
            }
            CHECK_EQ(a, b);
        }
    }
}
