#ifndef SLED_ASYNC_ASYNC_H
#define SLED_ASYNC_ASYNC_H

namespace sled {
class FiberScheduler;
}

namespace async {
sled::FiberScheduler &default_scheduler();
}

#define LIBASYNC_CUSTOM_DEFAULT_SCHEDULER
#include <async++.h>

namespace sled {
class FiberScheduler {
public:
    void schedule(async::task_run_handle t);
};

}// namespace sled

#endif// SLED_ASYNC_ASYNC_H
