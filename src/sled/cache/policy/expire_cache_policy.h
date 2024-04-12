#ifndef SLED_SLED_CACHE_POLICY_EXPIRE_CACHE_POLICY_H
#define SLED_SLED_CACHE_POLICY_EXPIRE_CACHE_POLICY_H

#pragma once
#include "abstract_cache_policy.h"
#include "sled/time_utils.h"
#include "sled/units/timestamp.h"
#include <map>

namespace sled {

template<typename TKey>
class ExpireCachePolicy : public AbstractCachePolicy<TKey> {
public:
    ~ExpireCachePolicy() override = default;

    void OnAdd(const TKey &key) override
    {
        Timestamp now = Timestamp::Nanos(TimeNanos());
        auto iter     = key_index_.insert(std::make_pair(now, key));
        keys_[key]    = iter;
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
        auto iter     = key_index_.begin();
        Timestamp now = Timestamp::Nanos(TimeNanos());
        while (iter != key_index_.end() && iter->first < now) {
            elems_to_remove.insert(iter->second);
            ++iter;
        }
    }

    bool IsValid(const TKey &key) override
    {
        auto iter = keys_.find(key);
        if (iter != keys_.end()) {
            return iter->second->first + expire_time_ > Timestamp::Nanos(TimeNanos());
        } else {
            return false;
        }
    }

private:
    TimeDelta expire_time_;
    std::multimap<Timestamp, TKey> key_index_;
    std::map<TKey, typename std::multimap<Timestamp, TKey>::iterator> keys_;
};
}// namespace sled

#endif// SLED_SLED_CACHE_POLICY_EXPIRE_CACHE_POLICY_H
