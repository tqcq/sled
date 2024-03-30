#include <sled/strings/base64.h>

TEST_SUITE("Base64")
{

    TEST_CASE("EncodedLength")
    {
        CHECK_EQ(0, sled::Base64::EncodedLength(0));
        CHECK_EQ(4, sled::Base64::EncodedLength(1));
        CHECK_EQ(4, sled::Base64::EncodedLength(2));
        CHECK_EQ(4, sled::Base64::EncodedLength(3));
        CHECK_EQ(8, sled::Base64::EncodedLength(4));
        CHECK_EQ(8, sled::Base64::EncodedLength(5));
        CHECK_EQ(8, sled::Base64::EncodedLength(6));
        CHECK_EQ(12, sled::Base64::EncodedLength(7));
    }

    TEST_CASE("DecodedLength")
    {
        CHECK_EQ(0, sled::Base64::DecodedLength(nullptr, 0));
        CHECK_EQ(1, sled::Base64::DecodedLength(nullptr, 1));
        CHECK_EQ(2, sled::Base64::DecodedLength(nullptr, 2));
        CHECK_EQ(2, sled::Base64::DecodedLength(nullptr, 3));
        CHECK_EQ(3, sled::Base64::DecodedLength(nullptr, 4));

        CHECK_EQ(0, sled::Base64::DecodedLength("", 0));
    }

    TEST_CASE("Encode")
    {
        CHECK_EQ("aGVsbG8gd29ybGQK", sled::Base64::Encode("hello world\n"));
        CHECK_EQ("U2VuZCByZWluZm9yY2VtZW50cwo=", sled::Base64::Encode("Send reinforcements\n"));
        CHECK_EQ("", sled::Base64::Encode(""));
        CHECK_EQ("IA==", sled::Base64::Encode(" "));
        CHECK_EQ("AA==", sled::Base64::Encode(std::string("\0", 1)));
        CHECK_EQ("AAA=", sled::Base64::Encode(std::string("\0\0", 2)));
        CHECK_EQ("AAAA", sled::Base64::Encode(std::string("\0\0\0", 3)));
    }

    TEST_CASE("Decode")
    {
        CHECK_EQ("hello world\n", sled::Base64::Decode("aGVsbG8gd29ybGQK").value());
        CHECK_EQ("Send reinforcements\n", sled::Base64::Decode("U2VuZCByZWluZm9yY2VtZW50cwo=").value());
        CHECK_EQ("", sled::Base64::Decode("").value());
        CHECK_EQ(" ", sled::Base64::Decode("IA==").value());
        CHECK_EQ(std::string("\0", 1), sled::Base64::Decode("AA==").value());
        CHECK_EQ(std::string("\0\0", 2), sled::Base64::Decode("AAA=").value());
        CHECK_EQ(std::string("\0\0\0", 3), sled::Base64::Decode("AAAA").value());
    }
}
