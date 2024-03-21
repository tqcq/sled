#include "sled/ref_count.h"
#include <gtest/gtest.h>
#include <sled/exec/just.h>

TEST(just, basic)
{
    sled::just_sender<int> sender = sled::just(1);
    auto op = connect(sender, sled::cout_receiver{});
}
