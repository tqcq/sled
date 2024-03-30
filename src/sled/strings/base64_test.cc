#include <gtest/gtest.h>
#include <sled/strings/base64.h>

TEST(Base64, EncodedLength)
{
    EXPECT_EQ(0, sled::Base64::EncodedLength(0));
    EXPECT_EQ(4, sled::Base64::EncodedLength(1));
    EXPECT_EQ(4, sled::Base64::EncodedLength(2));
    EXPECT_EQ(4, sled::Base64::EncodedLength(3));
    EXPECT_EQ(8, sled::Base64::EncodedLength(4));
    EXPECT_EQ(8, sled::Base64::EncodedLength(5));
    EXPECT_EQ(8, sled::Base64::EncodedLength(6));
    EXPECT_EQ(12, sled::Base64::EncodedLength(7));
}

TEST(Base64, DecodedLength)
{
    EXPECT_EQ(0, sled::Base64::DecodedLength(nullptr, 0));
    EXPECT_EQ(1, sled::Base64::DecodedLength(nullptr, 1));
    EXPECT_EQ(2, sled::Base64::DecodedLength(nullptr, 2));
    EXPECT_EQ(2, sled::Base64::DecodedLength(nullptr, 3));
    EXPECT_EQ(3, sled::Base64::DecodedLength(nullptr, 4));

    EXPECT_EQ(0, sled::Base64::DecodedLength("", 0));
}

TEST(Base64, Encode)
{
    EXPECT_EQ("aGVsbG8gd29ybGQK", sled::Base64::Encode("hello world\n"));
    EXPECT_EQ("U2VuZCByZWluZm9yY2VtZW50cwo=", sled::Base64::Encode("Send reinforcements\n"));
    EXPECT_EQ("", sled::Base64::Encode(""));
    EXPECT_EQ("IA==", sled::Base64::Encode(" "));
    EXPECT_EQ("AA==", sled::Base64::Encode(std::string("\0", 1)));
    EXPECT_EQ("AAA=", sled::Base64::Encode(std::string("\0\0", 2)));
    EXPECT_EQ("AAAA", sled::Base64::Encode(std::string("\0\0\0", 3)));
}

TEST(Base64, Decode)
{
    EXPECT_EQ("hello world\n", sled::Base64::Decode("aGVsbG8gd29ybGQK").value());
    EXPECT_EQ("Send reinforcements\n", sled::Base64::Decode("U2VuZCByZWluZm9yY2VtZW50cwo=").value());
    EXPECT_EQ("", sled::Base64::Decode("").value());
    EXPECT_EQ(" ", sled::Base64::Decode("IA==").value());
    EXPECT_EQ(std::string("\0", 1), sled::Base64::Decode("AA==").value());
    EXPECT_EQ(std::string("\0\0", 2), sled::Base64::Decode("AAA=").value());
    EXPECT_EQ(std::string("\0\0\0", 3), sled::Base64::Decode("AAAA").value());
}
