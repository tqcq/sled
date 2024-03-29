#include <gtest/gtest.h>
#include <sled/futures/detail/just.h>

TEST(Just, basic) { auto s1 = sled::detail::Just(42); }
