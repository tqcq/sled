#include <gtest/gtest.h>
#include <sled/strings/utils.h>

TEST(ToLower, Char)
{
    EXPECT_EQ(sled::ToLower('A'), 'a');
    EXPECT_EQ(sled::ToLower('Z'), 'z');
    EXPECT_EQ(sled::ToLower('a'), 'a');
    EXPECT_EQ(sled::ToLower('z'), 'z');
    EXPECT_EQ(sled::ToLower(' '), ' ');
    EXPECT_EQ(sled::ToLower('\0'), '\0');
}

TEST(ToUpper, Char)
{
    EXPECT_EQ(sled::ToUpper('A'), 'A');
    EXPECT_EQ(sled::ToUpper('Z'), 'Z');
    EXPECT_EQ(sled::ToUpper('a'), 'A');
    EXPECT_EQ(sled::ToUpper('z'), 'Z');
    EXPECT_EQ(sled::ToUpper(' '), ' ');
    EXPECT_EQ(sled::ToUpper('\0'), '\0');
}

TEST(ToLower, String)
{
    EXPECT_EQ(sled::ToLower("Hello World"), "hello world");
    EXPECT_EQ(sled::ToLower("HELLO WORLD"), "hello world");
    EXPECT_EQ(sled::ToLower("hello world"), "hello world");
    EXPECT_EQ(sled::ToLower(" "), " ");
    EXPECT_EQ(sled::ToLower(""), "");
}

TEST(ToUpper, String)
{
    EXPECT_EQ(sled::ToUpper("Hello World"), "HELLO WORLD");
    EXPECT_EQ(sled::ToUpper("HELLO WORLD"), "HELLO WORLD");
    EXPECT_EQ(sled::ToUpper("hello world"), "HELLO WORLD");
    EXPECT_EQ(sled::ToUpper(" "), " ");
    EXPECT_EQ(sled::ToUpper(""), "");
}

TEST(StrJoin, Empty)
{
    EXPECT_EQ(sled::StrJoin({}, ","), "");
    EXPECT_EQ(sled::StrJoin({}, ",", true), "");
    EXPECT_EQ(sled::StrJoin({}, ",", false), "");
}

TEST(StrJoin, Delim)
{
    EXPECT_EQ(sled::StrJoin({"a", "b", "c"}, ","), "a,b,c");
    EXPECT_EQ(sled::StrJoin({"a", "b", "c"}, ",", true), "a,b,c");
    EXPECT_EQ(sled::StrJoin({"a", "b", "c"}, ",", false), "a,b,c");
}

TEST(StrJoin, DelimSkipEmpty)
{
    EXPECT_EQ(sled::StrJoin({"a", "", "c"}, ","), "a,,c");
    EXPECT_EQ(sled::StrJoin({"a", "", "c"}, ",", true), "a,c");
    EXPECT_EQ(sled::StrJoin({"a", "", "c"}, ",", false), "a,,c");
}

TEST(StrSplit, Empty)
{
    EXPECT_EQ(sled::StrSplit("", ","), std::vector<std::string>());
    EXPECT_EQ(sled::StrSplit("", ",", true), std::vector<std::string>());
    EXPECT_EQ(sled::StrSplit("", ",", false), std::vector<std::string>());
}

TEST(StrSplit, Delim)
{
    // single delim
    EXPECT_EQ(sled::StrSplit("a,b,c", ","), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b,c", ",", false), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b,c,", ","), std::vector<std::string>({"a", "b", "c", ""}));
    EXPECT_EQ(sled::StrSplit("a,b,c,", ",", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b,c,", ",", false), std::vector<std::string>({"a", "b", "c", ""}));
    EXPECT_EQ(sled::StrSplit(",a,b,c", ","), std::vector<std::string>({"", "a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit(",a,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit(",a,b,c", ",", false), std::vector<std::string>({"", "a", "b", "c"}));

    // multi delim
    EXPECT_EQ(sled::StrSplit(",,a,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit(",,a,b,c", ",", false), std::vector<std::string>({"", "", "a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b,c,,", ",", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b,c,,", ",", false), std::vector<std::string>({"a", "b", "c", "", ""}));
    EXPECT_EQ(sled::StrSplit("a,,b,c", ",", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,,b,c", ",", false), std::vector<std::string>({"a", "", "b", "c"}));
}

TEST(StrSplit, MultiDelim)
{
    EXPECT_EQ(sled::StrSplit("a,b;c", ",;", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b;c", ",;", false), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b;c,", ",;", true), std::vector<std::string>({"a", "b", "c"}));
    EXPECT_EQ(sled::StrSplit("a,b;c,", ",;", false), std::vector<std::string>({"a", "b", "c", ""}));
    EXPECT_EQ(sled::StrSplit("a,b;c,", ";,", true), std::vector<std::string>({"a", "b", "c"}));
}
