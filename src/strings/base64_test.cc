#include <gtest/gtest.h>
#include <sled/strings/base64.h>

#define CONCAT_IMPL(A, B) A##B
#define CONCAT(A, B) CONCAT_IMPL(A, B)

#define TEST_ENCODE_DECODE(base64, text)                                                                               \
    do {                                                                                                               \
        ASSERT_EQ(sled::Base64::Encode(text), base64);                                                                 \
        auto CONCAT(res, __LINE__) = sled::Base64::Decode(base64);                                                     \
        ASSERT_TRUE(CONCAT(res, __LINE__).ok());                                                                       \
        ASSERT_EQ(CONCAT(res, __LINE__).value(), text);                                                                \
    } while (0)

TEST(Base64, EncodeAndDecode)
{
    TEST_ENCODE_DECODE("aGVsbG8gd29ybGQK", "hello world\n");
    TEST_ENCODE_DECODE("", "");
    TEST_ENCODE_DECODE("U2VuZCByZWluZm9yY2VtZW50cwo=", "Send reinforcements\n");
    TEST_ENCODE_DECODE(" ", " ");
    TEST_ENCODE_DECODE("AA==", std::string("\0", 1));
    TEST_ENCODE_DECODE("AAA=", std::string("\0\0", 2));
    TEST_ENCODE_DECODE("AAAA", std::string("\0\0\0", 3));
}
