#include <gtest/gtest.h>
#include <sled/debugging/symbolize.h>
#include <sled/make_unique.h>

void
TestFunc1()
{}

class Class {
public:
    Class() {}

    void MemberFunc1() {}

    int MemberFunc2() { return 0; }

    int MemberFunc3(int) { return 0; }
};

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

template<typename TRet, typename... Args>
void *
void_cast(TRet (*fn)(Args...))
{
    return reinterpret_cast<void *>(fn);
}

template<typename TClass, typename TRet, typename... Args>
void *
void_cast(TRet (TClass::*mem_func)(Args...))
{

    union {
        void *void_casted;
        TRet (TClass::*p)(Args...);
    };

    p = mem_func;
    return void_casted;
}

TEST(Symbolize, base)
{
    char buf[1024];
    EXPECT_STREQ("TestFunc1()", TrySymbolize(void_cast(TestFunc1)));
    EXPECT_STREQ("Class::MemberFunc1()", TrySymbolize(void_cast(&Class::MemberFunc1)));
    EXPECT_STREQ("Class::MemberFunc2()", TrySymbolize(void_cast(&Class::MemberFunc2)));
    EXPECT_STREQ("Class::MemberFunc3()", TrySymbolize(void_cast(&Class::MemberFunc3)));
    EXPECT_STREQ("TrySymbolizeWithLimit()", TrySymbolize(void_cast(&TrySymbolizeWithLimit)));
}

int
main(int argc, char *argv[])
{
    sled::InitializeSymbolizer(argv[0]);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
