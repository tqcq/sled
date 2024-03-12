#include <gtest/gtest.h>
#include <random>
#include <sled/system/thread_pool.h>

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-10, 10);
auto rnd = std::bind(dist, mt);

void
simulate_hard_computation()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(20 + rnd()));
}

// Simple function that adds multiplies two numbers and prints the result
void
multiply(const int a, const int b)
{
    simulate_hard_computation();
    const int res = a * b;
}

// Same as before but now we have an output parameter
void
multiply_output(int &out, const int a, const int b)
{
    simulate_hard_computation();
    out = a * b;
}

// Same as before but now we have an output parameter
int
multiply_return(const int a, const int b)
{
    simulate_hard_computation();
    const int res = a * b;
    return res;
}

class ThreadPoolTest : public ::testing::Test {
public:
    void SetUp() override { tp = new sled::ThreadPool(); }

    void TearDown() override { delete tp; }

    sled::ThreadPool *tp;
};

TEST_F(ThreadPoolTest, Output)
{
    for (int i = 0; i < 100; ++i) {
        int out;
        tp->submit(multiply_output, std::ref(out), i, i).get();
        EXPECT_EQ(out, i * i);
    }
}

TEST_F(ThreadPoolTest, Return)
{
    for (int i = 0; i < 100; ++i) {
        auto f = tp->submit(multiply_return, i, i);
        EXPECT_EQ(f.get(), i * i);
    }
}
