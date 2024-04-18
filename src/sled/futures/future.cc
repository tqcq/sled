
#include "sled/futures/future.h"
#include "sled/system/thread_pool.h"

namespace sled {
namespace future_detail {
void
IncrementFuturesUsage()
{}

void
DecrementFuturesUsage()
{}

}// namespace future_detail

static ThreadPool g_default_thread_pool;
TaskQueueBase *g_default_scheduler = &g_default_thread_pool;

void
SetDefaultScheduler(TaskQueueBase *scheduler) noexcept
{
    if (scheduler == nullptr) {
        g_default_scheduler = &g_default_thread_pool;
    } else {
        g_default_scheduler = scheduler;
    }
}

TaskQueueBase *
GetDefaultScheduler() noexcept
{
    return g_default_scheduler;
}

}// namespace sled
