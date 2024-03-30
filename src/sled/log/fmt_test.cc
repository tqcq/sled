#include <sled/log/log.h>

TEST_SUITE("fmt::format")
{

    TEST_CASE("enum")
    {
        enum EnumType {
            kOne   = 1,
            kTwo   = 2,
            kThree = 3,
        };

        std::stringstream ss;
        ss << kOne;

        CHECK_EQ(ss.str(), "1");
        CHECK_EQ(fmt::format("{}{}{}", kOne, kTwo, kThree), "123");
    }

    TEST_CASE("neg_enum")
    {
        enum EnumType {
            kOne   = -1,
            kTwo   = -2,
            kThree = -3,
        };

        CHECK_EQ(fmt::format("{}{}{}", kOne, kTwo, kThree), "-1-2-3");
    }

    struct Streamable {
        int value;
    };

    std::ostream &operator<<(std::ostream &os, const Streamable &s) { return os << s.value; }

    TEST_CASE("streamable")
    {
        Streamable s{42};
        CHECK_EQ(fmt::format("{}", s), "42");
    }
}
