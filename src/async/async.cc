#include "sled/async/async.h"
#include "sled/synchronization/event.h"
#include "sled/system/thread_pool.h"
#include "sled/utility/move_on_copy.h"
// clang-format off
#include <async++.h>


namespace sled {

void
SleepWaitHandler(async::task_wait_handle t)
{
    sled::Event event;
    t.on_finish([&] { event.Set(); });
    event.Wait(sled::Event::kForever);
}

FiberScheduler::FiberScheduler()
{
}

void
FiberScheduler::schedule(async::task_run_handle t)
{
    static ThreadPool thread_pool;
    auto move_on_copy = sled::MakeMoveOnCopy(t);
    thread_pool.submit([move_on_copy] { move_on_copy.value.run_with_wait_handler(SleepWaitHandler); });
    // thread_pool.submit([move_on_copy] { move_on_copy.value.run(); });
}

}// namespace sled

// clang-format on
namespace async {
sled::FiberScheduler &
default_scheduler()
{
    static sled::FiberScheduler scheduler;
    return scheduler;
}

void
detail::wait_for_task(task_base *wait_task)
{
    sled::SleepWaitHandler(task_wait_handle(wait_task));
}
}// namespace async
