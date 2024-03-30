#define SLED_TESTING_TEST_H
#include "sled/testing/doctest.h"

int
main(int argc, char *argv[])
{
    return doctest::Context(argc, argv).run();
}
