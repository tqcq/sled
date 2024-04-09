#include <sled/nonstd/expected.h>
#include <sled/status.h>

TEST_SUITE("expected")
{
    TEST_CASE("base")
    {
        sled::expected<int, int> e = 21;
        auto ret                   = e.map([](int v) { return v * 2; });
        CHECK(ret);
        CHECK_EQ(*ret, 42);
    }

    TEST_CASE("unexpect")
    {
        sled::expected<int, int> e{sled::unexpect, 0};
        CHECK_EQ(e.error(), 0);
    }
}
