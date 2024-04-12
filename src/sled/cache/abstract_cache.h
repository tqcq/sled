#ifndef SLED_CACHE_CACHE_H
#define SLED_CACHE_CACHE_H

#pragma once
#include "policy/abstract_cache_policy.h"
#include "sled/synchronization/mutex.h"

namespace sled {
template<typename TKey, typename TValue, typename TPolicy = AbstractCachePolicy<TKey>>
class AbstractCache {
    using Iterator = typename std::map<TKey, std::shared_ptr<TValue>>::iterator;

public:
    AbstractCache() { DoInitialize(); }

    virtual ~AbstractCache() { DoUninitialize(); }

    void Add(const TKey &key, const TValue &value)
    {
        sled::MutexLock lock(&mutex_);
        DoAdd(key, value);
    }

    void Update(const TKey &key, const TValue &value)
    {
        sled::MutexLock lock(&mutex_);
        DoUpdate(key, value);
    }

    void Add(const TKey &key, std::shared_ptr<TValue> value)
    {
        sled::MutexLock lock(&mutex_);
        DoAdd(key, value);
    }

    void Update(const TKey &key, std::shared_ptr<TValue> value)
    {
        sled::MutexLock lock(&mutex_);
        DoUpdate(key, value);
    }

    void Remove(const TKey &key)
    {
        sled::MutexLock lock(&mutex_);
        Iterator iter = data_.find(key);
        DoRemove(iter);
    }

    bool Has(const TKey &key)
    {
        sled::MutexLock lock(&mutex_);
        return DoHas(key);
    }

    std::shared_ptr<TValue> Get(const TKey &key)
    {
        sled::MutexLock lock(&mutex_);
        return DoGet(key);
    }

    void Clear()
    {
        sled::MutexLock lock(&mutex_);
        DoClear();
    }

    std::size_t size()
    {
        sled::MutexLock lock(&mutex_);
        DoReplace();
        return data_.size();
    }

    std::size_t empty() { return size() == 0; }

    void ForceReplace()
    {
        sled::MutexLock lock(&mutex_);
        DoReplace();
    }

    std::set<TKey> GetAllKeys()
    {
        sled::MutexLock lock(&mutex_);
        DoReplace();
        std::set<TKey> keys;
        for (const auto &pair : data_) { keys.insert(pair.first); }
        return keys;
    }

protected:
    void DoInitialize() {}

    void DoUninitialize() {}

    void DoAdd(const TKey &key, const TValue &value)
    {
        auto value_ptr = std::shared_ptr<TValue>(new TValue(value));
        DoAdd(key, value_ptr);
    }

    void DoAdd(const TKey &key, std::shared_ptr<TValue> value)
    {
        Iterator iter = data_.find(key);
        DoRemove(iter);
        for (auto &policy : policy_) { policy->OnAdd(key); }
        data_.insert(std::make_pair(key, value));
        DoReplace();
    }

    void DoUpdate(const TKey &key, const TValue &value)
    {
        auto value_ptr = std::shared_ptr<TValue>(new TValue(value));
        DoUpdate(key, value_ptr);
    }

    void DoUpdate(const TKey &key, std::shared_ptr<TValue> value)
    {
        Iterator iter = data_.find(key);
        if (iter != data_.end()) {
            for (auto &policy : policy_) { policy->OnRemove(iter->first); }
            iter->second = value;
        } else {
            for (auto &policy : policy_) { policy->OnAdd(key); }
            data_.insert(std::make_pair(key, value));
        }
        DoReplace();
    }

    void DoRemove(Iterator iter)
    {
        if (iter != data_.end()) {
            for (auto &policy : policy_) { policy->OnRemove(iter->first); }
            data_.erase(iter);
        }
    }

    bool DoHas(const TKey &key)
    {
        Iterator iter = data_.find(key);
        if (iter != data_.end()) {
            bool valid = true;
            for (auto &policy : policy_) { valid = policy->IsValid(key) && valid; }
            return valid;
        }
        return false;
    }

    std::shared_ptr<TValue> DoGet(const TKey &key)
    {
        Iterator iter = data_.find(key);
        if (iter != data_.end()) {
            for (auto &policy : policy_) { policy->OnGet(key); }
            bool valid = true;
            for (auto &policy : policy_) { valid = policy->IsValid(key) && valid; }
            if (!valid) {
                DoRemove(iter);
            } else {
                return iter->second;
            }
        }

        return nullptr;
    }

    void DoClear()
    {
        for (auto &policy : policy_) { policy->OnClear(); }
        data_.clear();
    }

    void DoReplace()
    {
        std::set<TKey> del_me;
        for (auto &policy : policy_) { policy->OnReplace(del_me); }
        for (const TKey &key : del_me) {
            auto iter = data_.find(key);
            DoRemove(iter);
        }
    }

protected:
    std::set<std::shared_ptr<TPolicy>> policy_;

private:
    sled::Mutex mutex_;
    std::map<TKey, std::shared_ptr<TValue>> data_;
};
}// namespace sled

#endif// SLED_CACHE_CACHE_H
