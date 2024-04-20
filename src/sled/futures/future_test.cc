#include <sled/futures/future.h>
#include <sled/system/thread.h>
#include <type_traits>

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

    TEST_CASE("throw")
    {
        sled::Promise<int, std::string> p;
        auto f = p.GetFuture().Map([](int x) {
            throw std::runtime_error("test");
            return x;
        });

        p.Success(42);
        REQUIRE(f.IsCompleted());
        REQUIRE(f.IsFailed());
        CHECK_EQ(f.FailureReason(), "test");
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

    TEST_CASE("Map")
    {
        sled::Promise<int, std::string> p;
        auto f  = p.GetFuture();
        auto f2 = f.Map([](int i) { return i + 1; })
                      .Map([](int i) { return std::to_string(i) + "00"; })
                      .Map([](const std::string &str) {
                          std::stringstream ss(str);
                          int x;
                          ss >> x;
                          return x;
                      });
        p.Success(42);
        CHECK(f2.Wait(-1));
        CHECK_EQ(f2.Result(), 4300);
    }

    TEST_CASE("FlatMap")
    {
        sled::Promise<int, std::string> p;
        auto f = p.GetFuture().FlatMap([](int i) {
            auto str = std::to_string(i);
            sled::Promise<std::string, std::string> p;
            p.Success(str);

            return p.GetFuture();
        });
        p.Success(42);
        CHECK(f.IsCompleted());
        CHECK_EQ(f.Result(), "42");
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

    TEST_CASE("Async")
    {
        auto f = sled::Future<int>::Async([]() { return 42; });
        CHECK_EQ(f.Result(), 42);

        auto f2 = sled::Future<int>::Async([]() {
            throw std::runtime_error("test");
            return 1;
        });
        CHECK_FALSE(f2.IsFailed());
        CHECK_EQ(std::string("test"), f2.FailureReason().what());
        CHECK(f2.IsFailed());

        auto f3 = sled::Future<std::string>::AsyncValue("11");
        CHECK_EQ(f3.Result(), "11");
    }

    TEST_CASE("Constructor")
    {
        sled::Future<int, std::string> f1 = 1;
        sled::Future<int, std::string> f2 = std::string("1");
        REQUIRE(f1.IsCompleted());
        REQUIRE(f2.IsFailed());
        REQUIRE_EQ(f1.Result(), 1);
        REQUIRE_EQ(f2.FailureReason(), "1");
    }

    TEST_CASE("MapFailure")
    {
        sled::Future<int, bool> f = false;
        auto f1                   = f.MapFailure([](bool) { return std::string("error"); });
        CHECK_EQ(f1.FailureReason(), "error");
    }

    TEST_CASE("Chain")
    {
        sled::Future<int, bool> f = 1;
        auto f1                   = f.Map([](int i) { return i + 1; })
                      .FlatMap([](int i) {
                          sled::Promise<std::string, bool> p;
                          p.Success(std::to_string(i));
                          return p.GetFuture();
                      })
                      .MapFailure([](bool) { return std::exception(); });
        CHECK_EQ(f1.Result(), "2");
    }
}
