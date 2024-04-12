#include <sled/cache/fifo_cache.h>

TEST_SUITE("FIFO Cache")
{
    TEST_CASE("Remove Oldest Key")
    {
        sled::FIFOCache<int, int> fifo_cache(3);
        CHECK(fifo_cache.empty());
        fifo_cache.Add(1, 1);
        CHECK_EQ(fifo_cache.size(), 1);
        fifo_cache.Add(2, 2);
        CHECK_EQ(fifo_cache.size(), 2);
        fifo_cache.Add(3, 3);
        CHECK_EQ(fifo_cache.size(), 3);

        CHECK(fifo_cache.Has(1));
        CHECK(fifo_cache.Has(2));
        CHECK(fifo_cache.Has(3));

        CHECK_EQ(*fifo_cache.Get(1), 1);
        CHECK_EQ(*fifo_cache.Get(2), 2);
        CHECK_EQ(*fifo_cache.Get(3), 3);

        fifo_cache.Add(4, 4);
        CHECK_EQ(fifo_cache.size(), 3);
        CHECK_FALSE(fifo_cache.Has(1));
    }
}
