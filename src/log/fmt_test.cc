#include <gtest/gtest.h>
#include <sled/log/log.h>

TEST(format, enum)
{
    enum EnumType {
        kOne = 1,
        kTwo = 2,
        kThree = 3,
    };

    EXPECT_EQ(fmt::format("{}{}{}", kOne, kTwo, kThree), "123");
}

TEST(format, neg_enum)
{
    enum EnumType {
        kOne = -1,
        kTwo = -2,
        kThree = -3,
    };

    EXPECT_EQ(fmt::format("{}{}{}", kOne, kTwo, kThree), "-1-2-3");
}
