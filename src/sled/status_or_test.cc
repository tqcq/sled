#include <gtest/gtest.h>
#include <sled/log/log.h>
#include <sled/status_or.h>

TEST(StatusOr, TestStatusOr)
{
    sled::StatusOr<int> so;
    EXPECT_FALSE(so.ok());
    so = sled::StatusOr<int>(1);
    EXPECT_TRUE(so.ok());
    EXPECT_EQ(so.value(), 1);
    EXPECT_EQ(so.status().code(), sled::StatusCode::kOk);
}

TEST(StatusOr, make_status_or)
{
    auto from_raw_str = sled::MakeStatusOr("hello");
    auto from_string  = sled::MakeStatusOr(std::string("world"));
    EXPECT_TRUE(from_raw_str.ok());
    EXPECT_TRUE(from_string.ok());
    EXPECT_EQ(from_raw_str.value(), "hello");
    EXPECT_EQ(from_string.value(), "world");
}
