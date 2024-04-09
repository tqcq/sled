#include <sled/config.h>
#include <sled/strings/utils.h>

static std::string test_config_name = "config_test";
static std::string test_config_path = sled::StripSuffix(__FILE__, "config_test.cc").to_string();

TEST_SUITE("Config")
{
    TEST_CASE("config")
    {
        sled::Config config;
        config.SetConfigName(test_config_name);
        config.AddConfigPath(test_config_path);
        config.SetDefault("top-string", "no effect");
        config.SetDefault("top-kk", "kk");
        CHECK(config.ReadInConfig());

        CHECK(config.IsSet("top-string"));
        CHECK_EQ(config.GetStringOr("top-string"), "bob");

        CHECK(config.IsSet("top-kk"));
        CHECK_EQ(config.GetStringOr("top-kk"), "kk");

        CHECK(config.IsSet("top-int"));
        CHECK(config.IsSet("top-bool"));
        CHECK_EQ(config.GetIntOr("top-int"), 10);
        CHECK_EQ(config.GetBoolOr("top-bool"), true);
        CHECK_EQ(config.GetIntOr("top-bool"), 1);

        CHECK(config.IsSet("top-table.key1"));
        CHECK_EQ(config.GetStringOr("top-table.key1"), "value1");
    }
}
