#include <sled/utility/move_on_copy.h>

TEST_SUITE("MoveOnCopy")
{
    TEST_CASE("Constructor imm value")
    {
        // immediately value
        sled::MoveOnCopy<int> m(1);

        auto m_copy = m;
        CHECK_EQ(m_copy.value, 1);
    }
    TEST_CASE("Constructor implicit convert")
    {
        sled::MoveOnCopy<std::string> from_str("test");
        auto from_str_copy = from_str;
        CHECK_EQ(from_str_copy.value, "test");
        CHECK_EQ(from_str.value, "");
    }

    TEST_CASE("Constructor from rvalue")
    {
        sled::MoveOnCopy<std::string> from_rvalue_std_string(std::move(std::string("test")));
        auto from_rvalue_std_string_copy = from_rvalue_std_string;
        CHECK_EQ(from_rvalue_std_string_copy.value, "test");
        CHECK_EQ(from_rvalue_std_string.value, "");
    }
    TEST_CASE("Constructor(&&)")
    {
        sled::MoveOnCopy<std::string> from_rvalue_std_string(std::move(std::string("test")));
        sled::MoveOnCopy<std::string> from_rvalue_std_string_copy(std::move(from_rvalue_std_string));
        CHECK_EQ(from_rvalue_std_string_copy.value, "test");
        CHECK_EQ(from_rvalue_std_string.value, "");
    }

    TEST_CASE("MakeMoveOnCopy")
    {
        auto m      = sled::MakeMoveOnCopy(std::move(std::string("test")));
        auto m_copy = m;
        CHECK_EQ(m_copy.value, "test");
        CHECK_EQ(m.value, "");
    }
}
