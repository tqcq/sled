#include <gtest/gtest.h>
#include <sled/futures/promise.h>

TEST(Promise, Basic)
{
    auto p = sled::Promise<int>();
    auto v = p.Then([](int v) {
                  EXPECT_EQ(v, 1);
                  return v + 10;
              })
                 .Tap([](int v) {
                     EXPECT_EQ(v, 11);
                     // no effect
                     return v + 1;
                 })
                 .Then([](int v) {
                     EXPECT_EQ(v, 11);
                     return v + 10;
                 });
    p.Resolve(1);
}

TEST(Future, Basic)
{
    auto p = sled::Promise<int>();
    auto future = p.GetFuture()
                      .Then([](int v) {
                          EXPECT_EQ(v, 1);
                          return v + 10;
                      })
                      .Then([](int v) {
                          EXPECT_EQ(v, 11);
                          return v + 10;
                      });
    p.Resolve(1);
    EXPECT_EQ(future.Get(), 21);
}
