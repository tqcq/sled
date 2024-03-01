/**
 * @file     : pending_task_safety_flag
 * @created  : Wednesday Feb 14, 2024 17:55:39 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_TASK_QUEUE_PENDING_TASK_SAFETY_FLAG_H
#define SLED_TASK_QUEUE_PENDING_TASK_SAFETY_FLAG_H

#include "sled/ref_counted_base.h"
#include "sled/scoped_refptr.h"

namespace sled {

class PendingTaskSafetyFlag final
    : public sled::RefCountedNonVirtual<PendingTaskSafetyFlag> {
public:
    static sled::scoped_refptr<PendingTaskSafetyFlag> Create();
    static sled::scoped_refptr<PendingTaskSafetyFlag> CreateDetached();
    static sled::scoped_refptr<PendingTaskSafetyFlag> CreateDetachedInactive();

    ~PendingTaskSafetyFlag() = default;
    void SetNotAlive();
    void SetAlive();
    bool alive() const;

protected:
    explicit PendingTaskSafetyFlag(bool alive) : alive_(alive) {}

private:
    static sled::scoped_refptr<PendingTaskSafetyFlag>
    CreateInternal(bool alive);
    bool alive_ = true;
};

}// namespace sled

#endif// SLED_TASK_QUEUE_PENDING_TASK_SAFETY_FLAG_H
