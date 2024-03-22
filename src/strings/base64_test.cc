#include <gtest/gtest.h>
#include <sled/strings/base64.h>

#define TEST_ENCODE_DECODE(base64, text)                                                                               \
    do {                                                                                                               \
        EXPECT_EQ(sled::Base64::Encode(text), std::string(base64));                                                    \
        auto res = sled::Base64::Decode(base64);                                                                       \
        EXPECT_TRUE(res.ok());                                                                                         \
        EXPECT_EQ(res.value(), text);                                                                                  \
    } while (0)

TEST(Base64, Encode) { EXPECT_EQ("aGVsbG8gd29ybGQK", sled::Base64::Encode("hello world\n")); }

TEST(Base64, Decode) { EXPECT_EQ("hello world\n", sled::Base64::Decode("aGVsbG8gd29ybGQK").value()); }

TEST(Base64, EncodeAndDecode)
{
    TEST_ENCODE_DECODE("aGVsbG8gd29ybGQK", "hello world\n");
    TEST_ENCODE_DECODE("U2VuZCByZWluZm9yY2VtZW50cwo=", "Send reinforcements\n");
    TEST_ENCODE_DECODE("", "");
    TEST_ENCODE_DECODE("IA==", " ");
    TEST_ENCODE_DECODE("AA==", std::string("\0", 1));
    TEST_ENCODE_DECODE("AAA=", std::string("\0\0", 2));
    TEST_ENCODE_DECODE("AAAA", std::string("\0\0\0", 3));
}
