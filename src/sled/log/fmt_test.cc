#include <gtest/gtest.h>
#include <sled/log/log.h>

TEST(format, enum)
{
    enum EnumType {
        kOne = 1,
        kTwo = 2,
        kThree = 3,
    };

    std::stringstream ss;
    ss << kOne;

    EXPECT_EQ(ss.str(), "1");
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

struct Streamable {
    int value;
};

std::ostream &
operator<<(std::ostream &os, const Streamable &s)
{
    return os << s.value;
}

TEST(format, streamable)
{
    Streamable s{42};
    EXPECT_EQ(fmt::format("{}", s), "42");
}
