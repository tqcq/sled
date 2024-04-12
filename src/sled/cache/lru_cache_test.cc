#include <sled/cache/cache.h>

TEST_SUITE("LRUCache")
{
    TEST_CASE("Remove Unused Key")
    {
        sled::LRUCache<int, int> lru_cache(3);
        CHECK(lru_cache.empty());

        lru_cache.Add(1, 1);
        CHECK_EQ(lru_cache.size(), 1);

        lru_cache.Add(2, 2);
        CHECK_EQ(lru_cache.size(), 2);

        lru_cache.Add(3, 3);
        CHECK_EQ(lru_cache.size(), 3);

        CHECK(lru_cache.Has(1));
        CHECK(lru_cache.Has(2));
        CHECK(lru_cache.Has(3));
        CHECK_EQ(*lru_cache.Get(3), 3);
        CHECK_EQ(*lru_cache.Get(2), 2);
        CHECK_EQ(*lru_cache.Get(1), 1);

        lru_cache.Add(4, 4);
        CHECK_EQ(lru_cache.size(), 3);
        CHECK_FALSE(lru_cache.Has(3));
    }
}
