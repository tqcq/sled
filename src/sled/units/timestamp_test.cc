#include <sled/units/timestamp.h>

TEST_SUITE("Timestamp")
{
    TEST_CASE("ToString")
    {
        CHECK_EQ(sled::ToString(sled::Timestamp::PlusInfinity()), "+inf ms");
        CHECK_EQ(sled::ToString(sled::Timestamp::MinusInfinity()), "-inf ms");
        CHECK_EQ(sled::ToString(sled::Timestamp::Micros(1)), "1 us");
        CHECK_EQ(sled::ToString(sled::Timestamp::Millis(1)), "1 ms");
        CHECK_EQ(sled::ToString(sled::Timestamp::Seconds(1)), "1 s");
    }

    TEST_CASE("IsExpired")
    {
        auto passed = sled::Timestamp::Now() - sled::TimeDelta::Millis(1);
        CHECK(passed.IsExpired());
        auto future = sled::Timestamp::Now() + sled::TimeDelta::Millis(1);
        CHECK_FALSE(future.IsExpired());
    }
}
