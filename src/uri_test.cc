#include <gtest/gtest.h>
#include <sled/uri.h>

TEST(URI, Absolute)
{
    sled::URI uri("http://example.com");
    EXPECT_EQ(uri.scheme(), "http");
    EXPECT_EQ(uri.host(), "example.com");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_TRUE(uri.query().empty());
    EXPECT_EQ(uri.anchor(), "");
}
