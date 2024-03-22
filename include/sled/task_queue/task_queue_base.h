/**
 * @file     : task_queue_base
 * @created  : Sunday Feb 04, 2024 20:55:06 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_TASK_QUEUE_TASK_QUEUE_BASE_H
#define SLED_TASK_QUEUE_TASK_QUEUE_BASE_H

#include "sled/system/location.h"
#include "sled/units/time_delta.h"
#include <functional>

namespace sled {

class TaskQueueBase {
public:
    enum class DelayPrecision {
        kLow,
        kHigh,
    };

    struct Deleter {
        void operator()(TaskQueueBase *task_queue) const { task_queue->Delete(); }
    };

    virtual void Delete() = 0;

    inline void PostTask(std::function<void()> &&task, const Location &location = Location::Current())
    {
        PostTaskImpl(std::move(task), PostTaskTraits{}, location);
    }

    inline void
    PostDelayedTask(std::function<void()> &&task, TimeDelta delay, const Location &location = Location::Current())
    {
        PostDelayedTaskImpl(std::move(task), delay, PostDelayedTaskTraits{}, location);
    }

    inline void PostDelayedHighPrecisionTask(std::function<void()> &&task,
                                             TimeDelta delay,
                                             const Location &location = Location::Current())
    {
        static PostDelayedTaskTraits traits(true);
        PostDelayedTaskImpl(std::move(task), delay, traits, location);
    }

    inline void PostDelayedTaskWithPrecision(DelayPrecision precision,
                                             std::function<void()> &&task,
                                             TimeDelta delay,
                                             const Location &location = Location::Current())
    {
        switch (precision) {
        case DelayPrecision::kLow:
            PostDelayedTask(std::move(task), delay, location);
            break;
        case DelayPrecision::kHigh:
            PostDelayedHighPrecisionTask(std::move(task), delay, location);
            break;
        }
    }

    void BlockingCall(std::function<void()> functor, const Location &location = Location::Current())
    {
        BlockingCallImpl(std::move(functor), location);
    }

    template<typename Functor,
             typename ReturnT = typename std::result_of<Functor()>::type,
             typename = typename std::enable_if<!std::is_void<ReturnT>::value, ReturnT>::type>
    ReturnT BlockingCall(Functor &&functor, const Location &location = Location::Current())
    {
        ReturnT result;
        BlockingCall([&] { result = std::forward<Functor>(functor)(); }, location);
        return result;
    }

    static TaskQueueBase *Current();

    bool IsCurrent() const { return Current() == this; };

protected:
    struct PostTaskTraits {};

    struct PostDelayedTaskTraits {
        PostDelayedTaskTraits(bool high_precision = false) : high_precision(high_precision) {}

        bool high_precision = false;
    };

    virtual void PostTaskImpl(std::function<void()> &&task, const PostTaskTraits &traits, const Location &location) = 0;
    virtual void PostDelayedTaskImpl(std::function<void()> &&task,
                                     TimeDelta delay,
                                     const PostDelayedTaskTraits &traits,
                                     const Location &location) = 0;
    virtual void BlockingCallImpl(std::function<void()> &&task, const Location &location);
    virtual ~TaskQueueBase() = default;

    class CurrentTaskQueueSetter {
    public:
        explicit CurrentTaskQueueSetter(TaskQueueBase *task_queue);
        ~CurrentTaskQueueSetter();
        CurrentTaskQueueSetter(const CurrentTaskQueueSetter &) = delete;
        CurrentTaskQueueSetter &operator=(const CurrentTaskQueueSetter &) = delete;

    private:
        TaskQueueBase *const previous_;
    };
};

}// namespace sled

#endif// SLED_TASK_QUEUE_TASK_QUEUE_BASE_H
