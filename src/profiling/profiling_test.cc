#include <gtest/gtest.h>
#include <sled/log/log.h>
#include <sled/profiling/profiling.h>

TEST(Profiling, GetProcessMemory)
{
    int rss, shared;
    sled::Profiling::GetProcessMemory(rss, shared);
    EXPECT_GT(rss, 0);
    EXPECT_GE(shared, 0);
}

TEST(Profiling, GetSystemMemory)
{
    int total_mem, available_mem, free_mem;
    sled::Profiling::GetSystemMemory(total_mem, available_mem, free_mem);
    EXPECT_GE(total_mem, 0);
    EXPECT_GE(available_mem, 0);
    EXPECT_GE(free_mem, 0);
}
