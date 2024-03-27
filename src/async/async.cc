#include "sled/async/async.h"
#include "sled/synchronization/event.h"
#include "sled/system/thread_pool.h"
#include "sled/utility/move_on_copy.h"
// clang-format off
#include <async++.h>

// clang-format on
namespace async {
sled::FiberScheduler &
default_scheduler()
{
    static sled::FiberScheduler scheduler;
    return scheduler;
}
}// namespace async

namespace sled {

void
SleepWaitHandler(async::task_wait_handle t)
{
    sled::Event event;
    t.on_finish([&] { event.Set(); });
    event.Wait(sled::Event::kForever);
}

void
FiberScheduler::schedule(async::task_run_handle t)
{
    static ThreadPool thread_pool;
    auto move_on_copy = sled::MakeMoveOnCopy(t);
    // thread_pool.PostTask([move_on_copy] { move_on_copy.value.run_with_wait_handler(SleepWaitHandler); });
    thread_pool.submit([move_on_copy] { move_on_copy.value.run_with_wait_handler(SleepWaitHandler); });
}

}// namespace sled
