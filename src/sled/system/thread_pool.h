#pragma once
#ifndef SLED_SYSTEM_THREAD_POOL_H
#define SLED_SYSTEM_THREAD_POOL_H
#include "sled/system/fiber/scheduler.h"
#include "sled/system/thread.h"
#include <functional>
#include <future>

namespace sled {
class ThreadPool final : public TaskQueueBase {
public:
    /**
    * @param num_threads The number of threads to create in the thread pool. If
    * -1, the number of threads will be equal to the number of hardware threads
    **/
    ThreadPool(int num_threads = -1);
    ~ThreadPool();

    template<typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        auto task_ptr                              = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        auto future                                = task_ptr->get_future();
        scheduler_->enqueue(marl::Task([task_ptr]() { (*task_ptr)(); }));
        return future;
    }

    void Delete() override;

protected:
    void PostTaskImpl(std::function<void()> &&task, const PostTaskTraits &traits, const Location &location) override;

    void PostDelayedTaskImpl(std::function<void()> &&task,
                             TimeDelta delay,
                             const PostDelayedTaskTraits &traits,
                             const Location &location) override;

private:
    sled::Scheduler *scheduler_;
    std::unique_ptr<sled::Thread> delayed_thread_;
};

}// namespace sled
#endif// SLED_SYSTEM_THREAD_POOL_H
