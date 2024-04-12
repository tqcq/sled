#include <sled/cache/expire_cache.h>
#include <sled/system/thread.h>

TEST_SUITE("Expire Cache")
{
    TEST_CASE("Remove Expired Key")
    {
        sled::ExpireCache<int, int> expire_cache(sled::TimeDelta::Millis(25));
        expire_cache.Add(1, 1);
        REQUIRE(expire_cache.Has(1));
        CHECK_EQ(*expire_cache.Get(1), 1);

        sled::Thread::SleepMs(30);
        CHECK_FALSE(expire_cache.Has(1));
        CHECK(expire_cache.empty());
        CHECK_EQ(expire_cache.Get(1), nullptr);
    }
}
