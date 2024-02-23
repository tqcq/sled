/**
 * @file     : task_queue_base
 * @created  : Sunday Feb 04, 2024 20:55:06 CST
 * @license  : MIT
 **/

#ifndef TASK_QUEUE_BASE_H
#define TASK_QUEUE_BASE_H

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
        void operator()(TaskQueueBase *task_queue) const
        {
            task_queue->Delete();
        }
    };

    virtual void Delete() = 0;

    void PostTask(std::function<void()> &&task,
                  const Location &location = Location::Current())
    {
        PostTaskImpl(std::move(task), PostTaskTraits{}, location);
    }

    void PostDelayedTask(std::function<void()> &&task,
                         TimeDelta delay,
                         const Location &location = Location::Current())
    {
        PostDelayedTaskImpl(std::move(task), delay, PostDelayedTaskTraits{},
                            location);
    }

    void
    PostDelayedHighPrecisionTask(std::function<void()> &&task,
                                 TimeDelta delay,
                                 const Location &location = Location::Current())
    {
        static PostDelayedTaskTraits traits(true);
        PostDelayedTaskImpl(std::move(task), delay, traits, location);
    }

    static TaskQueueBase *Current();

    bool IsCurrent() const { return Current() == this; };

protected:
    struct PostTaskTraits {};

    struct PostDelayedTaskTraits {
        PostDelayedTaskTraits(bool high_precision = false)
            : high_precision(high_precision)
        {}

        bool high_precision = false;
    };

    virtual void PostTaskImpl(std::function<void()> &&task,
                              const PostTaskTraits &traits,
                              const Location &location) = 0;
    virtual void PostDelayedTaskImpl(std::function<void()> &&task,
                                     TimeDelta delay,
                                     const PostDelayedTaskTraits &traits,
                                     const Location &location) = 0;
    virtual ~TaskQueueBase() = default;

    class CurrentTaskQueueSetter {
    public:
        explicit CurrentTaskQueueSetter(TaskQueueBase *task_queue);
        ~CurrentTaskQueueSetter();
        CurrentTaskQueueSetter(const CurrentTaskQueueSetter &) = delete;
        CurrentTaskQueueSetter &
        operator=(const CurrentTaskQueueSetter &) = delete;

    private:
        TaskQueueBase *const previous_;
    };
};

}// namespace sled

#endif// TASK_QUEUE_BASE_H
