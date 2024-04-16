#include <sled/futures/future.h>
#include <sled/system/thread.h>

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

    TEST_CASE("map")
    {
        sled::Promise<int, std::string> p;
        auto f  = p.GetFuture();
        auto f2 = f.Map([](int i) { return i + 1; });
        p.Success(42);
        CHECK(f2.Wait(-1));
        CHECK_EQ(f2.Result(), 43);
    }

    TEST_CASE("FlatMap")
    {
        // sled::Promise<int, std::string> p;
        // auto f = p.GetFuture().FlatMap([](int i) {
        //     auto str = std::to_string(i);
        //     sled::Promise<std::string, std::string> p;
        //     p.Success(str);
        //
        //     return p.GetFuture();
        // });
        // p.Success(42);
        // CHECK(f.Wait(-1));
        // CHECK_EQ(f.Result(), "42");
    }

    TEST_CASE("Via")
    {
        std::unique_ptr<sled::Thread> thread = sled::Thread::Create();
        thread->Start();

        std::thread::id tid      = thread->BlockingCall([]() { return std::this_thread::get_id(); });
        std::thread::id self_tid = std::this_thread::get_id();

        sled::Promise<int, std::string> p;
        auto f = p.GetFuture().Via(thread.get()).Map([](int i) { return std::this_thread::get_id(); });
        p.Success(42);

        CHECK_EQ(tid, f.Result());
        CHECK_NE(self_tid, f.Result());
    }
}
