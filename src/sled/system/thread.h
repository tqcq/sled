/**
 * @file     : thread
 * @created  : Sunday Feb 04, 2024 20:08:56 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_SYSTEM_THREAD_H
#define SLED_SYSTEM_THREAD_H
#include "sled/synchronization/mutex.h"
#include "sled/synchronization/thread_local.h"
#include "sled/task_queue/task_queue_base.h"
#include <atomic>
#include <memory>
#include <queue>
#include <thread>
#include <type_traits>

namespace sled {

class SocketServer;
class Thread;

class ThreadManager {
public:
    static const int kForever = -1;
    static ThreadManager *Instance();
    static void Add(Thread *message_queue);
    static void Remove(Thread *message_queue);
    Thread *CurrentThread();
    void SetCurrentThread(Thread *thread);
    Thread *WrapCurrentThread();
    void UnwrapCurrentThread();

private:
    void SetCurrentThreadInternal(Thread *message_queue);
    void AddInternal(Thread *message_queue);
    void RemoveInternal(Thread *message_queue);
    void ProcessAllMessageQueueInternal();
    void ProcessAllMessageQueuesForTesting();
    ThreadManager();
    ~ThreadManager();
    ThreadManager(const ThreadManager &) = delete;
    ThreadManager &operator=(const ThreadManager &) = delete;

    std::vector<Thread *> message_queues_;
    ThreadLocal<Thread *> current_thread_;
    Mutex cirt_;
};

class Thread : public TaskQueueBase {
public:
    static const int kForever = -1;
    explicit Thread(SocketServer *);
    explicit Thread(std::unique_ptr<SocketServer>);
    Thread(SocketServer *ss, bool do_init);
    Thread(std::unique_ptr<SocketServer> ss, bool do_init);
    ~Thread() override;
    Thread(const Thread &) = delete;
    Thread &operator=(const Thread &) = delete;

    static std::unique_ptr<Thread> CreateWithSocketServer();
    static std::unique_ptr<Thread> Create();
    static Thread *Current();

    SocketServer *socketserver();

    bool empty() const
    {
        MutexLock lock(&mutex_);
        return messages_.empty() && delayed_messages_.empty();
    }

    size_t size() const
    {
        MutexLock lock(&mutex_);
        return messages_.size() + delayed_messages_.size();
    }

    virtual void Quit();
    virtual bool IsQuitting();
    virtual void Restart();
    virtual void Stop();
    virtual void Run();
    void Delete() override;
    bool ProcessMessages(int cmsLoop);
    bool IsOwned() const;
    bool WrapCurrent();
    void UnwrapCurrent();
    // Amount of time until the next message can be retrieved.
    virtual int GetDelay();
    bool Start();
    void Join();
    bool IsCurrent() const;
    static bool SleepMs(int millis);

    const std::string &name() const { return name_; }

    bool SetName(const std::string &name, const void *obj);

protected:
    struct DelayedMessage {
        bool operator<(const DelayedMessage &dmsg) const
        {
            return (dmsg.run_time_ms < run_time_ms)
                || ((dmsg.run_time_ms == run_time_ms) && (dmsg.message_number < message_number));
        }

        int64_t delay_ms;
        int64_t run_time_ms;
        uint32_t message_number;
        mutable std::function<void()> functor;
    };

    void PostTaskImpl(std::function<void()> &&task, const PostTaskTraits &traits, const Location &location) override;
    void PostDelayedTaskImpl(std::function<void()> &&task,
                             TimeDelta delay,
                             const PostDelayedTaskTraits &traits,
                             const Location &location) override;
    void BlockingCallImpl(std::function<void()> &&functor, const Location &location) override;

    void DoInit();
    void DoDestroy();
    void WakeUpSocketServer();

private:
    std::function<void()> Get(int cmsWait);
    void Dispatch(std::function<void()> &&task);
    static void *PreRun(void *pv);
    bool WrapCurrentWithThreadManager(ThreadManager *thread_manager, bool need_synchronize_access);
    bool IsRunning();

    // for ThreadManager
    void EnsureIsCurrentTaskQueue();
    void ClearCurrentTaskQueue();

    mutable Mutex mutex_;
    std::queue<std::function<void()>> messages_ GUARDED_BY(mutex_);
    std::priority_queue<DelayedMessage> delayed_messages_ GUARDED_BY(mutex_);
    uint32_t delayed_next_num_ GUARDED_BY(mutex_);
    bool fInitialized_;
    bool fDestroyed_;
    std::atomic<int> stop_;
    SocketServer *const ss_;
    std::unique_ptr<SocketServer> own_ss_;
    std::string name_;
    std::unique_ptr<std::thread> thread_;
    bool owned_;

    std::unique_ptr<TaskQueueBase::CurrentTaskQueueSetter> task_queue_registration_;
    friend class ThreadManager;
};

class AutoSocketServerThread : public Thread {
public:
    explicit AutoSocketServerThread(SocketServer *ss);
    ~AutoSocketServerThread() override;

    AutoSocketServerThread(const AutoSocketServerThread &) = delete;
    AutoSocketServerThread &operator=(const AutoSocketServerThread &) = delete;

private:
    Thread *old_thread_;
};

}// namespace sled

#endif// SLED_SYSTEM_THREAD_H
