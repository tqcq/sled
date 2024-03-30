#include <gtest/gtest.h>
#include <sled/filesystem/path.h>

TEST(Path, TestCurrent)
{
    sled::Path path = sled::Path::Current();
    std::string str = path.ToString();
    EXPECT_FALSE(str.empty());
}

TEST(Path, TestHome)
{
    sled::Path path = sled::Path::Home();
    std::string str = path.ToString();
    EXPECT_FALSE(str.empty());
}

TEST(Path, TestTempDir)
{
    sled::Path path = sled::Path::TempDir();
    std::string str = path.ToString();
    EXPECT_FALSE(str.empty());
}
