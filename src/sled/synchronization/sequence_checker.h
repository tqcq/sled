/**
 * @file     : sequence_checker_internal
 * @created  : Saturday Feb 03, 2024 13:34:40 CST
 * @license  : MIT
 **/

#ifndef SLED_SYNCHRONIZATION_SEQUENCE_CHECKER_INTERNAL_H
#define SLED_SYNCHRONIZATION_SEQUENCE_CHECKER_INTERNAL_H
#pragma once

#include "sled/log/log.h"
#include "sled/synchronization/mutex.h"
#include "sled/task_queue/task_queue_base.h"

namespace sled {

class SLED_LOCKABLE SequenceChecker {
public:
    enum InitialState { kDetached = false, kAttached = true };

    explicit SequenceChecker(InitialState initial_state = kAttached);
    ~SequenceChecker() = default;

    bool IsCurrent() const;

    void Detach();

    // only use by macro RUN_ON
    std::string ExpectationToString() const;

private:
    mutable Mutex mutex_;
    mutable bool attached_ GUARDED_BY(mutex_);
    mutable pthread_t valid_thread_ GUARDED_BY(mutex_);
    mutable const TaskQueueBase *valid_queue_ GUARDED_BY(mutex_);
};

class SequenceCheckerDoNothing {
public:
    explicit SequenceCheckerDoNothing(bool attach_to_current_thread) {}

    bool IsCurrent() const { return true; }

    void Detach() {}
};

#define SLED_RUN_ON(x) THREAD_ANNOTATION_ATTRIBUTE__(exclusive_locks_required(x))
#define SLED_DCHECK_RUN_ON(x) SLED_DCHECK((x)->IsCurrent(), (x)->ExpectationToString())

}// namespace sled

#endif// SLED_SYNCHRONIZATION_SEQUENCE_CHECKER_INTERNAL_H
