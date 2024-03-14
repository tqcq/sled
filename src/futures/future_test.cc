#include <gtest/gtest.h>
#include <sled/futures/future.h>
#include <sled/futures/promise.h>

TEST(Future, Test1)
{
    sled::Promise<int> promise;
    sled::Future<int> future = promise.GetFuture();
    promise.SetValue(42);
}
