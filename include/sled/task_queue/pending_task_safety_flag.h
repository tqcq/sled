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
#include <functional>

namespace sled {

class PendingTaskSafetyFlag final : public sled::RefCountedNonVirtual<PendingTaskSafetyFlag> {
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
    static sled::scoped_refptr<PendingTaskSafetyFlag> CreateInternal(bool alive);
    bool alive_ = true;
};

class ScopedTaskSafety final {
public:
    ScopedTaskSafety() = default;

    explicit ScopedTaskSafety(scoped_refptr<PendingTaskSafetyFlag> flag) : flag_(std::move(flag)) {}

    ~ScopedTaskSafety() { flag_->SetNotAlive(); }

    void reset(scoped_refptr<PendingTaskSafetyFlag> new_flag = PendingTaskSafetyFlag::Create())
    {
        flag_->SetNotAlive();
        flag_ = std::move(new_flag);
    }

private:
    scoped_refptr<PendingTaskSafetyFlag> flag_;
};

inline std::function<void()>
SafeTask(scoped_refptr<PendingTaskSafetyFlag> flag, std::function<void()> task)
{
    return [flag, task]() mutable {
        if (flag->alive()) { std::move(task)(); }
    };
}

}// namespace sled

#endif// SLED_TASK_QUEUE_PENDING_TASK_SAFETY_FLAG_H
