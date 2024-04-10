#include <sled/nonstd/inja.h>

TEST_SUITE("inja")
{
    TEST_CASE("render")
    {
        inja::json data;
        data["a"] = 1;
        data["b"] = "str";
        auto res  = inja::render("{{ a }} {{ b }}", data);
        CHECK_EQ(res, "1 str");
    }
}
