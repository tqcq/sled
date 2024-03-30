#include <sled/debugging/demangle.h>
#include <sled/testing/test.h>

TEST_SUITE("Demangle")
{
    TEST_CASE("FunctionTemplate")
    {
        char tmp[100];

        // template <typename T>
        // int foo(T);
        //
        // foo<Wrapper<int>>({ .value = 5 });
        CHECK(sled::Demangle("_Z3fooIiEiT_", tmp, sizeof(tmp)));
        CHECK_EQ(doctest::toString(tmp), "foo<>()");
    }

    TEST_CASE("FunctionTemplateWithNesting")
    {
        char tmp[100];

        // template <std::integral T>
        // int foo(T);
        //
        // foo<int>(5);
        CHECK(sled::Demangle("_Z3fooI7WrapperIiEEiT_", tmp, sizeof(tmp)));
        CHECK_EQ(doctest::toString(tmp), "foo<>()");
    }

    TEST_CASE("FunctionTemplateWithNonTypeParamConstraint")
    {
        char tmp[100];

        // template <typename T>
        // int foo() requires std::integral<T>;
        //
        // foo<int>();
        CHECK(sled::Demangle("_Z3fooITkSt8integraliEiT_", tmp, sizeof(tmp)));
        CHECK_EQ(doctest::toString(tmp), "foo<>()");
    }

    TEST_CASE("AbiTags")
    {
        char tmp[100];

        // Mangled name generated via:
        // struct [[gnu::abi_tag("abc")]] A{};
        // A a;
        CHECK(sled::Demangle("_Z1aB3abc", tmp, sizeof(tmp)));
        CHECK_EQ(doctest::toString(tmp), "a[abi:abc]");

        CHECK(sled::Demangle("_ZN1BC2B3xyzEv", tmp, sizeof(tmp)));
        CHECK_EQ(doctest::toString(tmp), "B::B[abi:xyz]()");

        CHECK(sled::Demangle("_Z1CB3barB3foov", tmp, sizeof(tmp)));
        CHECK_EQ(doctest::toString(tmp), "C[abi:bar][abi:foo]()");
    }
}
