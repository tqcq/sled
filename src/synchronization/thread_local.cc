#include "sled/synchronization/thread_local.h"
#include <atomic>
#include <thread>
#include <unordered_map>

namespace sled {
namespace detail {

thread_local static std::unordered_map<ThreadId, std::unordered_map<ThreadLocalKey, void *>>
    thread_local_table_;
thread_local static std::atomic<ThreadLocalKey> next_key_;

class ThreadLocalManager::Impl final {
public:
    static ThreadId CurrentThreadId() { return std::this_thread::get_id(); }

    static ThreadLocalKey NextKey() { return next_key_.fetch_add(1); }

    void *Get(const ThreadId &thread_id, const ThreadLocalKey &key) const
    {
        auto iter = thread_local_table_.find(thread_id);
        if (iter == thread_local_table_.end()) { return nullptr; }
        auto &thread_local_map = iter->second;
        auto value_iter = thread_local_map.find(key);
        if (value_iter == thread_local_map.end()) { return nullptr; }
        return value_iter->second;
    }

    void Delete(const ThreadId &thread_id, const ThreadLocalKey &key)
    {
        auto iter = thread_local_table_.find(thread_id);
        if (iter == thread_local_table_.end()) { return; }
        auto &thread_local_map = iter->second;
        thread_local_map.erase(key);
    }

    void Set(const ThreadId &thread_id, const ThreadLocalKey &key, void *value)
    {
        auto iter = thread_local_table_.find(thread_id);
        if (iter == thread_local_table_.end()) {
            iter =
                thread_local_table_.emplace(thread_id, std::unordered_map<ThreadLocalKey, void *>())
                    .first;
        }
        auto &thread_local_map = iter->second;
        thread_local_map[key] = value;
    }

private:
};

ThreadLocalManager &
ThreadLocalManager::Instance()
{
    static ThreadLocalManager instance;
    return instance;
}

ThreadLocalManager::ThreadLocalManager() : impl_(new Impl()) {}

ThreadLocalManager::~ThreadLocalManager() = default;

ThreadId
ThreadLocalManager::CurrentThreadId()
{
    return Impl::CurrentThreadId();
}

ThreadLocalKey
ThreadLocalManager::NextKey()
{
    return Impl::NextKey();
}

void *
ThreadLocalManager::Get(const ThreadId &thread_id, const ThreadLocalKey &key) const
{
    return impl_->Get(thread_id, key);
}

void
ThreadLocalManager::Delete(const ThreadId &thread_id, const ThreadLocalKey &key)
{
    impl_->Delete(thread_id, key);
}

void
ThreadLocalManager::Set(const ThreadId &thread_id, const ThreadLocalKey &key, void *value)
{
    impl_->Set(thread_id, key, value);
}
}// namespace detail
}// namespace sled
