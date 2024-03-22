#include "sled/task_queue/task_queue_base.h"
#include "sled/synchronization/event.h"

namespace sled {
namespace {
thread_local TaskQueueBase *current = nullptr;
}

TaskQueueBase *
TaskQueueBase::Current()
{
    return current;
}

TaskQueueBase::CurrentTaskQueueSetter::CurrentTaskQueueSetter(TaskQueueBase *task_queue) : previous_(current)
{
    current = task_queue;
}

TaskQueueBase::CurrentTaskQueueSetter::~CurrentTaskQueueSetter() { current = previous_; }

void
TaskQueueBase::BlockingCallImpl(std::function<void()> &&functor, const sled::Location &from)
{
    Event done;
    PostTask([functor, &done] {
        functor();
        done.Set();
    });
    done.Wait(Event::kForever);
}

}// namespace sled
