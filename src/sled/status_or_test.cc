#include <sled/log/log.h>
#include <sled/status_or.h>

TEST_SUITE("StatusOr")
{
    TEST_CASE("StatusOr")
    {
        sled::StatusOr<int> so;
        CHECK_FALSE(so.ok());
        so = sled::StatusOr<int>(1);
        CHECK(so.ok());
        CHECK_EQ(so.value(), 1);
        CHECK_EQ(so.status().code(), sled::StatusCode::kOk);
    }

    TEST_CASE("MakeStatusOr")
    {
        auto from_raw_str = sled::MakeStatusOr("hello");
        auto from_string  = sled::MakeStatusOr(std::string("world"));
        CHECK(from_raw_str.ok());
        CHECK(from_string.ok());
        CHECK_EQ(from_raw_str.value(), "hello");
        CHECK_EQ(from_string.value(), "world");
    }
}
