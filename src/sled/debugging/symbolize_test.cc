#include <sled/debugging/symbolize.h>
#include <sled/make_unique.h>

void
TrivialFunc()
{}

static void
StaticFunc()
{}

class Class {
public:
    Class() {}

    static void StaticFunc() {}

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

// TODO: Support Linux
#if defined(__APPLE__)
TEST_SUITE("Symbolize")
{
    TEST_CASE("Trivial Function")
    {
        CHECK_EQ(doctest::String("TrivialFunc()"), TrySymbolize(void_cast(TrivialFunc)));
        CHECK_EQ(doctest::String("TrySymbolizeWithLimit()"), TrySymbolize(void_cast(&TrySymbolizeWithLimit)));
    }

    TEST_CASE("Static Function") { CHECK_EQ(doctest::String("StaticFunc()"), TrySymbolize(void_cast(StaticFunc))); }

    TEST_CASE("Member Function")
    {
        CHECK_EQ(doctest::String("Class::MemberFunc1()"), TrySymbolize(void_cast(&Class::MemberFunc1)));
        CHECK_EQ(doctest::String("Class::MemberFunc2()"), TrySymbolize(void_cast(&Class::MemberFunc2)));
        CHECK_EQ(doctest::String("Class::MemberFunc3()"), TrySymbolize(void_cast(&Class::MemberFunc3)));
    }

    TEST_CASE("Static Member Function")
    {
        CHECK_EQ(doctest::String("Class::StaticFunc()"), TrySymbolize(void_cast(&Class::StaticFunc)));
    }
}

#endif

int
main(int argc, char *argv[])
{
    sled::InitializeSymbolizer(argv[0]);
    return doctest::Context(argc, argv).run();
}
