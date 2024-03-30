#include <gtest/gtest.h>
#include <sled/uri.h>

TEST(URI, Absolute)
{
    sled::URI uri("http://example.com:1234/dir1/dir2/file?a=1#anchor");
    EXPECT_EQ(uri.scheme(), "http");
    EXPECT_EQ(uri.host(), "example.com");
    EXPECT_EQ(uri.port(), 1234);
    EXPECT_EQ(uri.path(), "/dir1/dir2/file");
    EXPECT_EQ(uri.query().size(), 1);
    EXPECT_EQ(uri.query()["a"], "1");
    EXPECT_EQ(uri.anchor(), "anchor");
}
