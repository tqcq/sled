#include <sled/system_time.h>
#include <sled/testing/benchmark.h>

static void
SystemTimeNanos(picobench::state &state)
{
    for (auto _ : state) { (void) sled::SystemTimeNanos(); }
}

PICOBENCH_SUITE("SystemTime");
PICOBENCH(SystemTimeNanos);
