#include <gtest/gtest.h>
#include <sled/log/log.h>
#include <sled/status.h>

TEST(Status, format)
{
    auto status = sled::Status(sled::StatusCode::kOk, "");
    EXPECT_EQ(fmt::format("{}", status), "OK");
}
