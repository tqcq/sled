#ifndef SLED_ASYNC_ASYNC_H
#define SLED_ASYNC_ASYNC_H

namespace sled {
class FiberScheduler;

}

namespace async {
sled::FiberScheduler &default_scheduler();

namespace detail {
class task_base;
void wait_for_task(task_base *wait_task);
}// namespace detail
}// namespace async

#define LIBASYNC_CUSTOM_WAIT_FOR_TASK
#define LIBASYNC_CUSTOM_DEFAULT_SCHEDULER
#include <async++.h>

namespace sled {
void SleepWaitHandler(async::task_wait_handle t);

class FiberScheduler {
public:
    FiberScheduler();
    void schedule(async::task_run_handle t);
};

}// namespace sled

#endif// SLED_ASYNC_ASYNC_H
