#include <gtest/gtest.h>
#include <sled/any.h>
#include <sled/log/log.h>

TEST(Any, Assign)
{
    sled::Any any1;
    sled::Any any2(any1);
    sled::Any any3(1);
    sled::Any any4(any3);
    sled::Any any5 = 1;
    EXPECT_FALSE(any1.HasValue());
    EXPECT_FALSE(any2.HasValue());
    EXPECT_TRUE(any3.HasValue());
    EXPECT_TRUE(any4.HasValue());
    EXPECT_TRUE(any5.HasValue());
    EXPECT_EQ(any3.Cast<int>(), 1);
    EXPECT_EQ(any4.Cast<int>(), 1);
    EXPECT_EQ(any5.Cast<int>(), 1);
    EXPECT_EQ(any3.CastOr<std::string>("def"), "def");
    EXPECT_EQ(any4.CastOr<std::string>("def"), "def");
    EXPECT_EQ(any5.CastOr<std::string>("def"), "def");
    EXPECT_EQ(any3.CastOr<int>(11), 1);
}

TEST(Any, std_swap)
{
    sled::Any a;
    sled::Any b = 2;
    EXPECT_FALSE(a.HasValue());
    EXPECT_TRUE(b.HasValue());
    std::swap(a, b);
    EXPECT_TRUE(a.HasValue());
    EXPECT_FALSE(b.HasValue());
    EXPECT_EQ(a.Cast<int>(), 2);
}

TEST(Any, custom_swap)
{
    sled::Any a;
    sled::Any b = 2;
    EXPECT_FALSE(a.HasValue());
    EXPECT_TRUE(b.HasValue());
    a.swap(b);
    EXPECT_TRUE(a.HasValue());
    EXPECT_FALSE(b.HasValue());
    EXPECT_EQ(a.Cast<int>(), 2);
}
