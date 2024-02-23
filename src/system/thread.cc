#include "sled/system/thread.h"
#include "sled/cleanup.h"
#include "sled/network/null_socket_server.h"
#include "sled/network/socket_server.h"
#include "sled/synchronization/event.h"
#include "sled/synchronization/thread_local.h"
#include "sled/time_utils.h"
#include <atomic>
#include <memory>
#include <thread>

namespace sled {

ThreadManager *
ThreadManager::Instance()
{
    static ThreadManager *const thread_manager = new ThreadManager();
    return thread_manager;
}

ThreadManager::ThreadManager() { current_thread_.Set(nullptr); }

ThreadManager::~ThreadManager() {}

void
ThreadManager::Add(Thread *message_queue)
{
    return Instance()->AddInternal(message_queue);
}

void
ThreadManager::AddInternal(Thread *message_queue)
{
    MutexLock lock(&cirt_);
    message_queues_.push_back(message_queue);
}

void
ThreadManager::Remove(Thread *message_queue)
{
    return Instance()->RemoveInternal(message_queue);
}

void
ThreadManager::RemoveInternal(Thread *message_queue)
{
    MutexLock lock(&cirt_);
    auto iter = std::find(message_queues_.begin(), message_queues_.end(),
                          message_queue);
    if (iter != message_queues_.end()) { message_queues_.erase(iter); }
}

Thread *
ThreadManager::CurrentThread()
{
    return current_thread_.Get();
}

void
ThreadManager::SetCurrentThread(Thread *thread)
{
    if (thread) {
        thread->EnsureIsCurrentTaskQueue();
    } else {
        Thread *current = CurrentThread();
        if (current) { current->ClearCurrentTaskQueue(); }
    }
    Instance()->SetCurrentThreadInternal(thread);
}

Thread *
ThreadManager::WrapCurrentThread()
{
    Thread *result = CurrentThread();
    if (result == nullptr) {
        result = new Thread(CreateDefaultSocketServer());
        result->WrapCurrentWithThreadManager(this, true);
    }
    return result;
}

void
ThreadManager::UnwrapCurrentThread()
{
    Thread *t = CurrentThread();
    if (t && !(t->IsOwned())) {
        t->UnwrapCurrent();
        delete t;
    }
}

void
ThreadManager::ProcessAllMessageQueueInternal()
{
    std::atomic<int> queues_not_done(0);
    MutexLock lock(&cirt_);
    for (Thread *queue : message_queues_) {
        queues_not_done.fetch_add(1);
        auto sub =
            MakeCleanup([&queues_not_done] { queues_not_done.fetch_sub(1); });
        queue->PostDelayedTask([&sub] {}, TimeDelta::Zero());
    }

    Thread *current = Thread::Current();
    while (queues_not_done.load() > 0) {
        if (current) { current->ProcessMessages(0); }
    }
}

void
ThreadManager::SetCurrentThreadInternal(Thread *message_queue)
{
    current_thread_.Set(message_queue);
}

Thread::Thread(SocketServer *ss) : Thread(ss, /*do_init=*/true) {}

Thread::Thread(std::unique_ptr<SocketServer> ss)
    : Thread(std::move(ss), /*do_init=*/true)
{}

Thread::Thread(SocketServer *ss, bool do_init)
    : delayed_next_num_(0),
      fInitialized_(false),
      fDestroyed_(false),
      stop_(0),
      ss_(ss)
{
    ss_->SetMessageQueue(this);
    SetName("Thread", this);
    if (do_init) { DoInit(); }
}

Thread::Thread(std::unique_ptr<SocketServer> ss, bool do_init)
    : Thread(ss.get(), do_init)
{
    own_ss_ = std::move(ss);
}

Thread::~Thread()
{
    Stop();
    DoDestroy();
}

void
Thread::DoInit()
{
    if (fInitialized_) { return; }

    fInitialized_ = true;
    ThreadManager::Add(this);
}

void
Thread::DoDestroy()
{
    if (fDestroyed_) { return; }
    fDestroyed_ = true;
    if (ss_) { ss_->SetMessageQueue(nullptr); }
    CurrentTaskQueueSetter set_current(this);
    messages_ = {};
    delayed_messages_ = {};
}

SocketServer *
Thread::socketserver()
{
    return ss_;
}

void
Thread::WakeUpSocketServer()
{
    ss_->WakeUp();
}

void
Thread::Quit()
{
    stop_.store(1, std::memory_order_release);
    WakeUpSocketServer();
}

bool
Thread::IsQuitting()
{
    return stop_.load(std::memory_order_acquire) != 0;
}

void
Thread::Restart()
{
    stop_.store(0, std::memory_order_release);
}

void
Thread::Stop()
{
    Thread::Quit();
    Join();
}

void
Thread::Run()
{
    ProcessMessages(kForever);
}

std::function<void()>
Thread::Get(int cmsWait)
{
    int64_t cmsTotal = cmsWait;
    int64_t cmsElapsed = 0;
    int64_t msStart = TimeMillis();
    int64_t msCurrent = msStart;

    while (true) {
        int64_t cmsDelayNext = kForever;
        {
            MutexLock lock(&mutex_);
            // check delayed_messages_
            while (!delayed_messages_.empty()) {
                int64_t first_run_time_ms = delayed_messages_.top().run_time_ms;
                if (msCurrent < first_run_time_ms) {
                    cmsDelayNext = TimeDiff(first_run_time_ms, msCurrent);
                    break;
                }
                messages_.push(std::move(delayed_messages_.top().functor));
                delayed_messages_.pop();
            }
            // check messages_
            if (!messages_.empty()) {
                std::function<void()> task = std::move(messages_.front());
                messages_.pop();
                return std::move(task);
            }
        }

        if (IsQuitting()) { break; }

        int64_t cmsNext;
        if (cmsWait == kForever) {
            cmsNext = cmsDelayNext;
        } else {
            cmsNext = std::max<int64_t>(0, cmsTotal - cmsElapsed);
            if ((cmsDelayNext != kForever) && (cmsDelayNext < cmsNext)) {
                cmsNext = cmsDelayNext;
            }
        }
        {
            if (!ss_->Wait(cmsNext == kForever ? SocketServer::kForever
                                               : TimeDelta::Millis(cmsNext),
                           /*process_io=*/true)) {
                return nullptr;
            }
        }

        msCurrent = TimeMillis();
        cmsElapsed = TimeDiff(msCurrent, msStart);
        if (cmsWait != kForever) {
            if (cmsElapsed >= cmsWait) { return nullptr; }
        }
    }
    return nullptr;
}

void
Thread::PostTaskImpl(std::function<void()> &&task,
                     const PostTaskTraits &traits,
                     const Location &location)
{
    if (IsQuitting()) { return; }
    {
        MutexLock lock(&mutex_);
        messages_.push(std::move(task));
    }
    WakeUpSocketServer();
}

void
Thread::PostDelayedTaskImpl(std::function<void()> &&task,
                            TimeDelta delay,
                            const PostDelayedTaskTraits &traits,
                            const Location &location)
{
    if (IsQuitting()) { return; }

    int64_t delay_ms = delay.RoundUpTo(TimeDelta::Millis(1)).ms<int>();
    int64_t run_time_ms = TimeAfterMillis(delay_ms);
    {
        MutexLock lock(&mutex_);
        delayed_messages_.push({.delay_ms = delay_ms,
                                .run_time_ms = run_time_ms,
                                .message_number = delayed_next_num_,
                                .functor = std::move(task)

        });
        ++delayed_next_num_;
        // assert delayed_next_num_ != 0
    }
    WakeUpSocketServer();
}

void
Thread::BlockingCallImpl(std::function<void()> functor,
                         const Location &location)
{
    if (IsQuitting()) { return; }
    if (IsCurrent()) {
        functor();
        return;
    }

    Thread *current_thread = Thread::Current();
    Event done;
    PostTask([functor, &done] {
        functor();
        done.Set();
    });
    done.Wait(Event::kForever);
}

int
Thread::GetDelay()
{
    MutexLock lock(&mutex_);
    if (!messages_.empty()) { return 0; }

    if (!delayed_messages_.empty()) {
        int delay = TimeUntilMillis(delayed_messages_.top().run_time_ms);
        return std::max<int>(0, delay);
    }

    return kForever;
}

void
Thread::Dispatch(std::function<void()> &&task)
{
    int64_t start_time = TimeMillis();
    std::move(task)();
    int64_t end_time = TimeMillis();
    int64_t diff = TimeDiff(end_time, start_time);
}

bool
Thread::SleepMs(int milliseconds)
{
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (milliseconds % 1000) * 1000000;
    int ret = nanosleep(&ts, nullptr);
    if (ret != 0) {
        // error log
        return false;
    }
    return true;
}

bool
Thread::SetName(const std::string &name, const void *obj)
{
    name_ = name;
    if (obj) {
        char buf[30];
        snprintf(buf, sizeof(buf), " 0x%p", obj);
        name_ += buf;
    }
    return true;
}

void
Thread::EnsureIsCurrentTaskQueue()
{
    task_queue_registration_.reset(
        new TaskQueueBase::CurrentTaskQueueSetter(this));
}

void
Thread::ClearCurrentTaskQueue()
{
    task_queue_registration_.reset();
}

bool
Thread::Start()
{
    if (IsRunning()) { return false; }

    Restart();
    ThreadManager::Instance();
    owned_ = true;
    thread_.reset(new std::thread(&Thread::PreRun, this));
    return true;
}

void
Thread::Join()
{
    if (!IsRunning()) { return; }
    thread_->join();
    thread_.reset();
}

bool
Thread::IsCurrent() const
{
    return ThreadManager::Instance()->CurrentThread() == this;
}

bool
Thread::IsRunning()
{
    return thread_ != nullptr;
}

void *
Thread::PreRun(void *pv)
{
    Thread *thread = static_cast<Thread *>(pv);
    ThreadManager::Instance()->SetCurrentThread(thread);
    thread->Run();
    ThreadManager::Instance()->SetCurrentThread(nullptr);
    return nullptr;
}

bool
Thread::WrapCurrentWithThreadManager(ThreadManager *thread_manager,
                                     bool need_synchronize_access)
{
    // assert(!IsRunning());
    owned_ = false;
    thread_manager->SetCurrentThread(this);
    return true;
}

bool
Thread::IsOwned() const
{
    return owned_;
}

bool
Thread::WrapCurrent()
{
    return WrapCurrentWithThreadManager(ThreadManager::Instance(), true);
}

void
Thread::UnwrapCurrent()
{
    ThreadManager::Instance()->SetCurrentThread(nullptr);
    thread_.reset();
}

void
Thread::Delete()
{
    Stop();
    delete this;
}

bool
Thread::ProcessMessages(int cmsLoop)
{
    int64_t msEnd = kForever == cmsLoop ? 0 : TimeAfterMillis(cmsLoop);
    int64_t cmsNext = cmsLoop;
    while (true) {
        auto task = Get(cmsNext);
        if (!task) { return !IsQuitting(); }
        Dispatch(std::move(task));
        if (cmsLoop != kForever) {
            cmsNext = static_cast<int>(TimeUntilMillis(msEnd));
            if (cmsNext < 0) { return true; }
        }
    }
}

/* static */ std::unique_ptr<Thread>
Thread::CreateWithSocketServer()
{
    return std::unique_ptr<Thread>(new Thread(CreateDefaultSocketServer()));
}

/* static */ std::unique_ptr<Thread>
Thread::Create()
{
    auto socket_server = std::unique_ptr<SocketServer>(new NullSocketServer);
    return std::unique_ptr<Thread>(new Thread(std::move(socket_server)));
}

Thread *
Thread::Current()
{
    ThreadManager *manager = ThreadManager::Instance();
    Thread *thread = manager->CurrentThread();
    return thread;
}

AutoSocketServerThread::AutoSocketServerThread(SocketServer *ss)
    : Thread(ss, /*do_init=*/false)
{
    DoInit();
    old_thread_ = ThreadManager::Instance()->CurrentThread();
    ThreadManager::Instance()->SetCurrentThread(nullptr);
    ThreadManager::Instance()->SetCurrentThread(this);
    if (old_thread_) { ThreadManager::Remove(old_thread_); }
}

AutoSocketServerThread::~AutoSocketServerThread()
{
    Stop();
    DoDestroy();
    ThreadManager::Instance()->SetCurrentThread(nullptr);
    ThreadManager::Instance()->SetCurrentThread(old_thread_);
    if (old_thread_) { ThreadManager::Add(old_thread_); }
}

}// namespace sled
