#include "sled/task_queue/pending_task_safety_flag.h"
#include "sled/scoped_refptr.h"

namespace sled {
sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::CreateInternal(bool alive)
{
    // Explicit new, to access private constructor.
    return sled::scoped_refptr<PendingTaskSafetyFlag>(new PendingTaskSafetyFlag(alive));
}

sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::Create()
{
    return CreateInternal(true);
}

sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::CreateDetached()
{
    scoped_refptr<PendingTaskSafetyFlag> safety_flag = CreateInternal(true);
    safety_flag->main_sequence_.Detach();
    return safety_flag;
}

sled::scoped_refptr<PendingTaskSafetyFlag>
PendingTaskSafetyFlag::CreateDetachedInactive()
{
    scoped_refptr<PendingTaskSafetyFlag> safety_flag = CreateInternal(false);
    safety_flag->main_sequence_.Detach();
    return safety_flag;
}

void
PendingTaskSafetyFlag::SetNotAlive()
{
    SLED_DCHECK_RUN_ON(&main_sequence_);
    alive_ = false;
}

void
PendingTaskSafetyFlag::SetAlive()
{
    SLED_DCHECK_RUN_ON(&main_sequence_);
    alive_ = true;
}

bool
PendingTaskSafetyFlag::alive() const
{
    SLED_DCHECK_RUN_ON(&main_sequence_);
    return alive_;
}

}// namespace sled
