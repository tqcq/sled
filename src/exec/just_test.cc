#include <gtest/gtest.h>
#include <sled/exec/exec.h>

struct cout_receiver {
    template<typename T>
    void SetValue(T &&val)
    {
        // 这个receiver什么都不干，只对收集到的结果输出
        std::cout << "Result: " << val << '\n';
    }

    void SetError(std::exception_ptr err) { std::terminate(); }

    void SetStopped() { std::terminate(); }
};

TEST(Just, basic)
{
    sled::Just(42).Connect(cout_receiver{}).Start();
    sled::Just(11).Connect(cout_receiver{}).Start();
}

TEST(Then, basic)
{
    auto s1 = sled::Just(42);
    auto s2 = sled::Then(s1, [](int x) { return x + 1; });
    auto s3 = sled::Then(s2, [](int x) { return x + 1; });
    auto s4 = sled::Then(s3, [](int x) { return x + 1; });
    s4.Connect(cout_receiver{}).Start();
}

TEST(SyncWait, basic)
{
    auto s1 = sled::Just(42);
    auto s2 = sled::Then(s1, [](int x) { return x + 1; });
    auto s3 = sled::Then(s2, [](int x) { return x + 1; });
    auto s4 = sled::Then(s3, [](int x) { return x + 1; });
    auto s5 = sled::SyncWait(s4).value();
    std::cout << "Result: " << s5 << '\n';
}
