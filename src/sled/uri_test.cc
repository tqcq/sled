#include <sled/log/log.h>
#include <sled/uri.h>

TEST_SUITE("URI")
{
    TEST_CASE("Base")
    {
        sled::URI uri("http://user:pass@example.com:1234/dir1/dir2/file?a=1&b=1#anchor");
        CHECK_EQ(uri.scheme(), "http");
        CHECK_EQ(uri.host(), "example.com");
        CHECK_EQ(uri.port(), 1234);
        CHECK_EQ(uri.path(), "/dir1/dir2/file");
        CHECK_EQ(uri.query(), "a=1&b=1");
        CHECK_EQ(uri.query_param()["a"], "1");
        CHECK_EQ(uri.anchor(), "anchor");
    }
    TEST_CASE("scheme://authority - With domain")
    {
        auto host_and_port = sled::URI("http://baidu.com:443");
        CHECK_EQ(host_and_port.scheme(), "http");
        CHECK_EQ(host_and_port.host(), "baidu.com");
        CHECK_EQ(host_and_port.port(), 443);

        auto host = sled::URI("http://baidu.com");
        CHECK_EQ(host_and_port.scheme(), "http");
        CHECK_EQ(host.host(), "baidu.com");

        auto host2 = sled::URI("http://baidu.com/");
        CHECK_EQ(host_and_port.scheme(), "http");
        CHECK_EQ(host2.host(), "baidu.com");
        CHECK_EQ(host2.path(), "/");

        auto user_info = sled::URI("http://user:pass@example.com");
        CHECK_EQ(user_info.scheme(), "http");
        CHECK_EQ(user_info.username(), "user");
        CHECK_EQ(user_info.password(), "pass");
        CHECK_EQ(user_info.host(), "example.com");

        auto user_info2 = sled::URI("http://a_.!~*\'(-)n0123Di%25%26:pass;:&=+$,word@www.zend.com");
        CHECK_EQ(user_info2.scheme(), "http");
        CHECK_EQ(user_info2.username(), "a_.!~*\'(-)n0123Di%25%26");
        CHECK_EQ(user_info2.password(), "pass;:&=+$,word");
        CHECK_EQ(user_info2.host(), "www.zend.com");
    }

    TEST_CASE("scheme://auhtority - With IPV6")
    {
        auto ipv6 = sled::URI("http://[::1]:443");
        CHECK_EQ(ipv6.scheme(), "http");
        CHECK_EQ(ipv6.host(), "[::1]");
        CHECK_EQ(ipv6.port(), 443);
        auto ipv6_2 = sled::URI("http://[::1]");
        CHECK_EQ(ipv6_2.scheme(), "http");
        CHECK_EQ(ipv6_2.host(), "[::1]");
        auto ipv6_3 = sled::URI("http://[::1]/");
        CHECK_EQ(ipv6_3.scheme(), "http");
        CHECK_EQ(ipv6_3.host(), "[::1]");
        CHECK_EQ(ipv6_3.path(), "/");
    }

    TEST_CASE("scheme:/")
    {
        auto root = sled::URI("http:/");
        CHECK_EQ(root.scheme(), "http");
        CHECK_EQ(root.path(), "/");
    }

    TEST_CASE("scheme:///")
    {
        auto root = sled::URI("http:///");
        CHECK_EQ(root.scheme(), "http");
        CHECK_EQ(root.path(), "/");
    }

    TEST_CASE("?queryOnly")
    {
        CHECK_EQ(sled::URI("?a=1&b=2").query(), "a=1&b=2");
        CHECK_EQ(sled::URI("?").query(), "");
    }

    TEST_CASE("#fragmentOnly")
    {
        CHECK_EQ(sled::URI("#anchor").anchor(), "anchor");
        CHECK_EQ(sled::URI("#").anchor(), "");
    }

    TEST_CASE("authorityOnly")
    {
        CHECK_EQ(sled::URI("//bob@example.com").host(), "example.com");
        CHECK_EQ(sled::URI("//bob@example.com").username(), "bob");
        CHECK_EQ(sled::URI("mailto:bob@example.com").scheme(), "mailto");
        CHECK_EQ(sled::URI("mailto:bob@example.com").path(), "bob@example.com");
        CHECK_EQ(sled::URI("example.com").path(), "example.com");
        CHECK_EQ(sled::URI("example.com:").scheme(), "example.com");
        CHECK_EQ(sled::URI("example.com:1234").scheme(), "example.com");
        CHECK_EQ(sled::URI("example.com:1234").path(), "1234");
    }

    TEST_CASE("invliad uri")
    {
        // auto invalid  = sled::URI("http:");
        // auto invalid2 = sled::URI("http://");
        // auto invalid3 = sled::URI("http://");
        // auto invalid4 = sled::URI("[[::1]]");
        CHECK_EQ(sled::URI("[[2620:0:1cfe:face:b00c::3]]").host(), "");
        CHECK_EQ(sled::URI("[[2620:0:1cfe:face:b00c::3]]").path(), "[[2620:0:1cfe:face:b00c::3]]");
    }

    TEST_CASE("href")
    {
        CHECK_EQ(sled::URI("http://example.com").href(), "http://example.com");
        CHECK_EQ(sled::URI("http://example.com:1234").href(), "http://example.com:1234");
        CHECK_EQ(sled::URI("http://example.com:1234/").href(), "http://example.com:1234/");
        CHECK_EQ(sled::URI("http://example.com:1234/dir1/dir2/file?a=1&b=1#anchor").href(),
                 "http://example.com:1234/dir1/dir2/file?a=1&b=1#anchor");
        CHECK_EQ(sled::URI("mailto:bob@example.com").href(), "mailto:bob@example.com");
        CHECK_EQ(sled::URI("bob@example.com").href(), "bob@example.com");
        CHECK_EQ(sled::URI("bob@example.com:999").href(), "bob@example.com:999");
    }
}
