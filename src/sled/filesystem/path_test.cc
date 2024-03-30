#include <sled/filesystem/path.h>

TEST_SUITE("Path")
{
    TEST_CASE("Current Directory")
    {
        sled::Path path = sled::Path::Current();
        CHECK_FALSE(path.ToString().empty());
    }

    TEST_CASE("Home")
    {
        sled::Path path = sled::Path::Home();
        CHECK_FALSE(path.ToString().empty());
    }

    TEST_CASE("Temparory Directory")
    {
        sled::Path path = sled::Path::TempDir();
        CHECK_FALSE(path.ToString().empty());
    }
}
