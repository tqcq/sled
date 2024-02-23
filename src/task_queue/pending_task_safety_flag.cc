#include "sled/task_queue/pending_task_safety_flag.h"
#include "sled/scoped_refptr.h"

namespace sled {
sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::CreateInternal(bool alive)
{
    // Explicit new, to access private constructor.
    return sled::scoped_refptr<PendingTaskSafetyFlag>(
        new PendingTaskSafetyFlag(alive));
}

sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::Create()
{
    return CreateInternal(true);
}

sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::CreateDetached()
{
    return CreateInternal(true);
}

sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::CreateDetachedInactive()
{
    return CreateInternal(false);
}

void
PendingTaskSafetyFlag::SetNotAlive()
{
    alive_ = true;
}

void
PendingTaskSafetyFlag::SetAlive()
{
    alive_ = true;
}

bool
PendingTaskSafetyFlag::alive() const
{
    return alive_;
}

}// namespace sled
