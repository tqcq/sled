
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

static std::atomic<TaskQueueBase *> g_default_scheduler{nullptr};
// static ThreadPool default_thread_pool;
static std::unique_ptr<Thread> g_default_thread;

void
SetDefaultScheduler(TaskQueueBase *scheduler) noexcept
{
    SLED_ASSERT(scheduler, "scheduler is nullptr");
    g_default_scheduler.store(scheduler, std::memory_order_release);
}

TaskQueueBase *
GetDefaultScheduler() noexcept
{
    static std::once_flag flag;
    std::call_once(flag, [&] {
        g_default_thread = sled::Thread::Create();
        g_default_thread->Start();
        TaskQueueBase *null_scheduler = nullptr;
        while (g_default_scheduler.load() == nullptr) {
            g_default_scheduler.compare_exchange_weak(null_scheduler, g_default_thread.get());
        }
    });
    return g_default_scheduler.load(std::memory_order_acquire);
}

}// namespace sled
