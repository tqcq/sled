#include <sled/log/log.h>
#include <sled/status.h>

TEST_SUITE("Status")
{
    TEST_CASE("format")
    {
        auto status = sled::Status(sled::StatusCode::kOk, "");
        CHECK_EQ(fmt::format("{}", status), "OK");
    }
}
