#include <sled/random.h>
#include <sled/strings/base64.h>
#include <sled/testing/benchmark.h>
#include <sstream>

static std::string
RandomString(size_t length)
{
    static const char chars[]
        = "0123456789"
          "abcdefghijklmnopqrstuvwxyz"
          "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::stringstream ss;
    sled::Random rand(1314);
    while (length--) { ss << chars[rand.Rand(sizeof(chars))]; }
    return ss.str();
}

static void
Base64Encode(picobench::state &state)
{
    for (auto _ : state) {
        state.pause_timer();
        std::string input = RandomString(10000);
        state.resume_timer();
        (void) sled::Base64::Encode(input);
    }
}

static void
Base64Decode(picobench::state &state)
{
    for (auto _ : state) {
        state.pause_timer();
        std::string input        = RandomString(10000);
        std::string base64_input = sled::Base64::Encode(input);
        state.resume_timer();
        (void) sled::Base64::Decode(base64_input);
    }
}

PICOBENCH(Base64Decode);
PICOBENCH(Base64Encode);
