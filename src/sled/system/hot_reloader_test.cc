#include <sled/sled.h>
#include <sled/system/hot_reloader.h>
#ifndef TEST_BIN_PATH
#error "must define TEST_BIN_PATH"
#endif

TEST_SUITE("Hot Loader")
{
    TEST_CASE("DynamicLibraryName")
    {
#if defined(_WIN32)
        CHECK_EQ(sled::DynamicLibraryName("test"), "test.dll");
#elif defined(__APPLE__)
        CHECK_EQ(sled::DynamicLibraryName("test"), "libtest.dylib");
#elif defined(__linux__)
        CHECK_EQ(sled::DynamicLibraryName("test"), "libtest.so");
#endif
    }

    TEST_CASE("crash_test")
    {
        sled::HotReloader reloader(sled::DynamicLibraryName("hot_reloader_test_dynamic"));
        CHECK_MESSAGE(reloader.Initialize(), "Initialize failed, ", reloader.name());
        CHECK_EQ(reloader.version(), 0);
        CHECK_EQ(reloader.next_version(), 1);
        CHECK_EQ(reloader.last_working_version(), 0);

        int op;
        reloader.set_userdata(&op);

        SUBCASE("OnLoad Crash")
        {
            // OnLoad crash test
            op                                               = 0;
            sled::optional<sled::HotReloader::Failure> error = reloader.UpdateOrError();
            REQUIRE(error);
            CHECK_EQ(*error, sled::HotReloader::Failure::kSegmentationFault);
            CHECK_EQ(op, -1);

            CHECK_EQ(reloader.version(), 0);
            CHECK_EQ(reloader.next_version(), 2);
            CHECK_EQ(reloader.last_working_version(), 0);
        }
        SUBCASE("OnClose crash")
        {
            // OnClose crash test
            op                                               = 3;
            sled::optional<sled::HotReloader::Failure> error = reloader.UpdateOrError();
            REQUIRE_FALSE(error);
            CHECK_EQ(op, -1);
            CHECK_EQ(reloader.version(), 1);
            CHECK_EQ(reloader.next_version(), 2);
            CHECK_EQ(reloader.last_working_version(), 0);
        }
        SUBCASE("OnClose crash")
        {
            // OnStep crash test
            op                                               = 1;
            sled::optional<sled::HotReloader::Failure> error = reloader.UpdateOrError();
            REQUIRE_FALSE(error);
            CHECK_EQ(op, -1);
        }
    }
}
