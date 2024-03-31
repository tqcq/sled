#include <random>
#include <sled/synchronization/event.h>
#include <sled/system/thread_pool.h>

std::random_device rd;
std::mt19937 mt(rd());
std::uniform_int_distribution<int> dist(-5, 5);
auto rnd = std::bind(dist, mt);

void
simulate_hard_computation()
{
    // std::this_thread::sleep_for(std::chrono::milliseconds(10 + rnd()));
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

TEST_SUITE("ThreadPool")
{
    TEST_CASE("submit")
    {
        sled::ThreadPool *tp = new sled::ThreadPool();
        REQUIRE_NE(tp, nullptr);

        SUBCASE("Output")
        {
            for (int i = 0; i < 100; ++i) {
                int out;
                tp->submit(multiply_output, std::ref(out), i, i).get();
                CHECK_EQ(out, i * i);
            }
        }
        SUBCASE("Return")
        {
            for (int i = 0; i < 100; ++i) {
                auto f = tp->submit(multiply_return, i, i);
                CHECK_EQ(f.get(), i * i);
            }
        }

        delete tp;
    }
    TEST_CASE("PostTask")
    {
        sled::ThreadPool *tp = new sled::ThreadPool();
        sled::Event waiter;
        tp->PostTask([&]() { waiter.Set(); });
        CHECK(waiter.Wait(sled::TimeDelta::Seconds(1)));
        delete tp;
    }

    TEST_CASE("PostDelayedTask")
    {
        sled::ThreadPool *tp = new sled::ThreadPool();
        sled::Event waiter;
        tp->PostDelayedTask([&]() { waiter.Set(); }, sled::TimeDelta::Millis(100));
        CHECK_FALSE(waiter.Wait(sled::TimeDelta::Millis(50)));
        CHECK(waiter.Wait(sled::TimeDelta::Millis(150)));
        delete tp;
    }
}
