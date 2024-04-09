#include <sled/nonstd/string_view.h>

using namespace sled;
using size_type = size_t;

TEST_SUITE("string_view")
{
    TEST_CASE("string_view: Allows to default construct an empty string_view")
    {
        string_view sv;

        // use parenthesis with data() to prevent lest from using sv.data() as C-string:

        CHECK(sv.size() == size_type(0));
        CHECK((sv.data() == nssv_nullptr));
    }

    TEST_CASE("string_view: Allows to construct from pointer and size")
    {
        string_view sv("hello world", 5);

        CHECK(sv.size() == size_type(5));
        CHECK(*(sv.data() + 0) == 'h');
        CHECK(*(sv.data() + 4) == 'o');
    }

    TEST_CASE("string_view: Allows to construct from C-string")
    {
        string_view sv("hello world");

        CHECK(sv.size() == size_type(11));
        CHECK(*(sv.data() + 0) == 'h');
        CHECK(*(sv.data() + 10) == 'd');
    }

    TEST_CASE("string_view: Allows to copy-construct from empty string_view")
    {
        string_view sv1;

        string_view sv2(sv1);

        // use parenthesis with data() to prevent lest from using sv.data() as C-string:

        CHECK(sv2.size() == size_type(0));
        CHECK((sv2.data() == nssv_nullptr));
    }

    TEST_CASE("string_view: Allows to copy-construct from non-empty string_view")
    {
        string_view sv1("hello world", 5);

        string_view sv2(sv1);

        CHECK(sv2.size() == sv1.size());
        CHECK((sv2.data() == sv1.data()));
        CHECK(*(sv2.data() + 0) == 'h');
        CHECK(*(sv2.data() + 4) == 'o');
    }
}
