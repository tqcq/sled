#include "sled/task_queue/task_queue_base.h"

namespace sled {
namespace {
thread_local TaskQueueBase *current = nullptr;
}

TaskQueueBase *
TaskQueueBase::Current()
{
    return current;
}

TaskQueueBase::CurrentTaskQueueSetter::CurrentTaskQueueSetter(TaskQueueBase *task_queue)
    : previous_(current)
{
    current = task_queue;
}

TaskQueueBase::CurrentTaskQueueSetter::~CurrentTaskQueueSetter() { current = previous_; }

}// namespace sled
