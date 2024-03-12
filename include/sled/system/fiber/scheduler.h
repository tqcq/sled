#pragma once
#ifndef SLED_SYSTEM_FIBER_SCHEDULER_H
#define SLED_SYSTEM_FIBER_SCHEDULER_H
#include <marl/defer.h>
#include <marl/scheduler.h>
#include <marl/task.h>

namespace sled {
using Scheduler = marl::Scheduler;

// schedule() schedules the task T to be asynchronously called using the
// currently bound scheduler.
inline void
Schedule(marl::Task &&t)
{
    MARL_ASSERT_HAS_BOUND_SCHEDULER("marl::schedule");
    auto scheduler = marl::Scheduler::get();
    scheduler->enqueue(std::move(t));
}

// schedule() schedules the function f to be asynchronously called with the
// given arguments using the currently bound scheduler.
template<typename Function, typename... Args>
inline void
Schedule(Function &&f, Args &&...args)
{
    MARL_ASSERT_HAS_BOUND_SCHEDULER("marl::schedule");
    auto scheduler = marl::Scheduler::get();
    scheduler->enqueue(marl::Task(
        std::bind(std::forward<Function>(f), std::forward<Args>(args)...)));
}

// schedule() schedules the function f to be asynchronously called using the
// currently bound scheduler.
template<typename Function>
inline void
Schedule(Function &&f)
{
    MARL_ASSERT_HAS_BOUND_SCHEDULER("marl::schedule");
    auto scheduler = marl::Scheduler::get();
    scheduler->enqueue(marl::Task(std::forward<Function>(f)));
}
}// namespace sled

#endif//  SLED_SYSTEM_FIBER_SCHEDULER_H
