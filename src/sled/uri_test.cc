#include <sled/uri.h>

TEST_CASE("")
{
    sled::URI uri("http://example.com:1234/dir1/dir2/file?a=1#anchor");
    CHECK_EQ(uri.scheme(), "http");
    CHECK_EQ(uri.host(), "example.com");
    CHECK_EQ(uri.port(), 1234);
    CHECK_EQ(uri.path(), "/dir1/dir2/file");
    CHECK_EQ(uri.query().size(), 1);
    CHECK_EQ(uri.query()["a"], "1");
    CHECK_EQ(uri.anchor(), "anchor");
}
