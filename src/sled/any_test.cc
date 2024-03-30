#include <sled/any.h>
#include <sled/log/log.h>

TEST_SUITE("Any")
{
    TEST_CASE("Assign")
    {
        sled::Any any1;
        sled::Any any2(any1);
        sled::Any any3(1);
        sled::Any any4(any3);
        sled::Any any5 = 1;
        CHECK_FALSE(any1.HasValue());
        CHECK_FALSE(any2.HasValue());
        CHECK(any3.HasValue());
        CHECK(any4.HasValue());
        CHECK(any5.HasValue());
        CHECK_EQ(any3.Cast<int>(), 1);
        CHECK_EQ(any4.Cast<int>(), 1);
        CHECK_EQ(any5.Cast<int>(), 1);
        CHECK_EQ(any3.CastOr<std::string>("def"), "def");
        CHECK_EQ(any4.CastOr<std::string>("def"), "def");
        CHECK_EQ(any5.CastOr<std::string>("def"), "def");
        CHECK_EQ(any3.CastOr<int>(11), 1);
    }

    TEST_CASE("std_swap")
    {
        sled::Any a;
        sled::Any b = 2;
        CHECK_FALSE(a.HasValue());
        CHECK(b.HasValue());
        std::swap(a, b);
        CHECK(a.HasValue());
        CHECK_FALSE(b.HasValue());
        CHECK_EQ(a.Cast<int>(), 2);
    }

    TEST_CASE("custom_swap")
    {
        sled::Any a;
        sled::Any b = 2;
        CHECK_FALSE(a.HasValue());
        CHECK(b.HasValue());
        a.swap(b);
        CHECK(a.HasValue());
        CHECK_FALSE(b.HasValue());
        CHECK_EQ(a.Cast<int>(), 2);
    }
}
