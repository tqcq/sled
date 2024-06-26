#ifndef SLED_SLED_CACHE_POLICY_EXPIRE_CACHE_POLICY_H
#define SLED_SLED_CACHE_POLICY_EXPIRE_CACHE_POLICY_H

#pragma once
#include "abstract_cache_policy.h"
#include "sled/units/timestamp.h"
#include <map>

namespace sled {

template<typename TKey>
class ExpireCachePolicy : public AbstractCachePolicy<TKey> {
public:
    ExpireCachePolicy(const TimeDelta &expire_time) : expire_time_(expire_time) {}

    ~ExpireCachePolicy() override = default;

    void OnAdd(const TKey &key) override
    {
        Timestamp cur_expire_time = Timestamp::Now() + expire_time_;
        auto iter                 = key_index_.insert(std::make_pair(cur_expire_time, key));
        keys_[key]                = iter;
    }

    void OnRemove(const TKey &key) override
    {
        auto iter = keys_.find(key);
        if (iter != keys_.end()) {
            key_index_.erase(iter->second);
            keys_.erase(iter);
        }
    }

    void OnGet(const TKey &key) override
    {
        // no change
    }

    void OnClear() override
    {
        key_index_.clear();
        keys_.clear();
    }

    void OnReplace(std::set<TKey> &elems_to_remove) override
    {
        auto iter = key_index_.begin();
        while (iter != key_index_.end() && iter->first.IsExpired()) {
            elems_to_remove.insert(iter->second);
            ++iter;
        }
    }

    bool IsValid(const TKey &key) override
    {
        auto iter = keys_.find(key);
        return iter != keys_.end() && !iter->second->first.IsExpired();
    }

private:
    const TimeDelta expire_time_;
    std::multimap<Timestamp, TKey> key_index_;
    std::map<TKey, typename std::multimap<Timestamp, TKey>::iterator> keys_;
};
}// namespace sled

#endif// SLED_SLED_CACHE_POLICY_EXPIRE_CACHE_POLICY_H
