#include "sled/system/thread_pool.h"
#include "sled/system/location.h"
#include "sled/task_queue/task_queue_base.h"

namespace sled {
ThreadPool::ThreadPool(int num_threads) : delayed_thread_(sled::Thread::Create())
{
    if (num_threads == -1) { num_threads = std::thread::hardware_concurrency(); }
    scheduler_ = new sled::Scheduler(sled::Scheduler::Config().setWorkerThreadCount(num_threads));
    delayed_thread_->Start();
}

ThreadPool::~ThreadPool() { delete scheduler_; }

void
ThreadPool::Delete()
{}

void
ThreadPool::PostTaskImpl(std::function<void()> &&task, const PostTaskTraits &traits, const Location &location)
{
    scheduler_->enqueue(marl::Task([task] { task(); }));
}

void
ThreadPool::PostDelayedTaskImpl(std::function<void()> &&task,
                                TimeDelta delay,
                                const PostDelayedTaskTraits &traits,
                                const Location &location)
{
    auto move_task_to_fiber = [task]() { task(); };
    if (traits.high_precision) {
        delayed_thread_->PostDelayedTaskWithPrecision(
            TaskQueueBase::DelayPrecision::kHigh,
            std::move(move_task_to_fiber),
            delay,
            location);
    } else {
        delayed_thread_->PostDelayedTaskWithPrecision(
            TaskQueueBase::DelayPrecision::kLow,
            std::move(move_task_to_fiber),
            delay,
            location);
    }
}

}// namespace sled
