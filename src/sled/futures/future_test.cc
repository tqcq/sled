#include <sled/futures/future.h>

TEST_SUITE("future")
{
    TEST_CASE("base success")
    {
        sled::Promise<int, std::string> p;
        auto f = p.GetFuture();
        p.Success(42);
        CHECK(f.Wait(-1));
        CHECK(f.IsValid());
        CHECK_EQ(f.Result(), 42);
    }
    TEST_CASE("base failed")
    {
        sled::Promise<int, std::string> p;
        auto f = p.GetFuture();
        p.Failure("error");
        REQUIRE(p.IsFilled());
        REQUIRE(f.IsCompleted());
        CHECK(f.Wait(-1));
        CHECK(f.IsValid());
        CHECK_EQ(f.FailureReason(), "error");
    }
    TEST_CASE("thread success") {}
}
