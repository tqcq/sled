#include <sled/strings/utils.h>

TEST_SUITE("String Utils")
{

    TEST_CASE("Char")
    {
        for (char c = 'A'; c <= 'Z'; ++c) { CHECK_EQ(sled::ToLower(c), c + 32); }
        for (char c = 'a'; c <= 'z'; ++c) { CHECK_EQ(sled::ToLower(c), c); }
        CHECK_EQ(sled::ToLower(' '), ' ');
        CHECK_EQ(sled::ToLower('\0'), '\0');
    }

    TEST_CASE("Char")
    {
        for (char c = 'A'; c <= 'Z'; ++c) { CHECK_EQ(sled::ToUpper(c), c); }
        for (char c = 'a'; c <= 'z'; ++c) { CHECK_EQ(sled::ToUpper(c), c - 32); }
        CHECK_EQ(sled::ToUpper(' '), ' ');
        CHECK_EQ(sled::ToUpper('\0'), '\0');
    }

    TEST_CASE("String")
    {
        CHECK_EQ(sled::ToLower("Hello World"), "hello world");
        CHECK_EQ(sled::ToLower("HELLO WORLD"), "hello world");
        CHECK_EQ(sled::ToLower("hello world"), "hello world");
        CHECK_EQ(sled::ToLower(" "), " ");
        CHECK_EQ(sled::ToLower(""), "");
    }

    TEST_CASE("String")
    {
        CHECK_EQ(sled::ToUpper("Hello World"), "HELLO WORLD");
        CHECK_EQ(sled::ToUpper("HELLO WORLD"), "HELLO WORLD");
        CHECK_EQ(sled::ToUpper("hello world"), "HELLO WORLD");
        CHECK_EQ(sled::ToUpper(" "), " ");
        CHECK_EQ(sled::ToUpper(""), "");
    }

    TEST_CASE("Empty")
    {
        CHECK_EQ(sled::StrJoin({}, ","), "");
        CHECK_EQ(sled::StrJoin({}, ",", true), "");
        CHECK_EQ(sled::StrJoin({}, ",", false), "");
    }

    TEST_CASE("Delim")
    {
        CHECK_EQ(sled::StrJoin({"a", "b", "c"}, ","), "a,b,c");
        CHECK_EQ(sled::StrJoin({"a", "b", "c"}, ",", true), "a,b,c");
        CHECK_EQ(sled::StrJoin({"a", "b", "c"}, ",", false), "a,b,c");
    }

    TEST_CASE("DelimSkipEmpty")
    {
        CHECK_EQ(sled::StrJoin({"a", "", "c"}, ","), "a,,c");
        CHECK_EQ(sled::StrJoin({"a", "", "c"}, ",", true), "a,c");
        CHECK_EQ(sled::StrJoin({"a", "", "c"}, ",", false), "a,,c");
    }

    TEST_CASE("Empty")
    {
        CHECK_EQ(sled::StrSplit("", ","), std::vector<std::string>());
        CHECK_EQ(sled::StrSplit("", ",", true), std::vector<std::string>());
        CHECK_EQ(sled::StrSplit("", ",", false), std::vector<std::string>());
    }

    TEST_CASE("Delim")
    {
        // single delim
        CHECK_EQ(sled::StrSplit("a,b,c", ","), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b,c", ",", false), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b,c,", ","), std::vector<std::string>({"a", "b", "c", ""}));
        CHECK_EQ(sled::StrSplit("a,b,c,", ",", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b,c,", ",", false), std::vector<std::string>({"a", "b", "c", ""}));
        CHECK_EQ(sled::StrSplit(",a,b,c", ","), std::vector<std::string>({"", "a", "b", "c"}));
        CHECK_EQ(sled::StrSplit(",a,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit(",a,b,c", ",", false), std::vector<std::string>({"", "a", "b", "c"}));

        // multi delim
        CHECK_EQ(sled::StrSplit(",,a,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit(",,a,b,c", ",", false), std::vector<std::string>({"", "", "a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b,c,,", ",", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b,c,,", ",", false), std::vector<std::string>({"a", "b", "c", "", ""}));
        CHECK_EQ(sled::StrSplit("a,,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,,b,c", ",", false), std::vector<std::string>({"a", "", "b", "c"}));
    }

    TEST_CASE("MultiDelim")
    {
        CHECK_EQ(sled::StrSplit("a,b;c", ",;", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b;c", ",;", false), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b;c,", ",;", true), std::vector<std::string>({"a", "b", "c"}));
        CHECK_EQ(sled::StrSplit("a,b;c,", ",;", false), std::vector<std::string>({"a", "b", "c", ""}));
        CHECK_EQ(sled::StrSplit("a,b;c,", ";,", true), std::vector<std::string>({"a", "b", "c"}));
    }
}
