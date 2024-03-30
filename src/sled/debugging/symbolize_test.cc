#include <gtest/gtest.h>
#include <sled/debugging/symbolize.h>
#include <sled/make_unique.h>

void
TestFunc1()
{}

static char try_symbolize_buffer[4096];

static const char *
TrySymbolizeWithLimit(void *pc, int limit)
{
    // ASSERT_LE(limit, 4096);
    auto heap_buffer = sled::MakeUnique<char[]>(4096);
    bool found       = sled::Symbolize(pc, heap_buffer.get(), sizeof(try_symbolize_buffer));
    strncpy(try_symbolize_buffer, heap_buffer.get(), sizeof(try_symbolize_buffer) - 1);
    try_symbolize_buffer[sizeof(try_symbolize_buffer) - 1] = '\0';

    return try_symbolize_buffer;
}

static const char *
TrySymbolize(void *pc)
{
    return TrySymbolizeWithLimit(pc, 4096);
}

// TEST(Symbolize, base)
// {
//     char buf[1024];
//     EXPECT_EQ("TestFunc1", TrySymbolize((void *) &TestFunc1));
// }

TEST(Symbolize, ReadAddrMap) { sled::ReadAddrMap(); }

int
main(int argc, char *argv[])
{
    sled::InitializeSymbolizer(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
