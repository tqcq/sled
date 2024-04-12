#ifndef SLED_SLED_CACHE_POLICY_LRU_CACHE_POLICY_H
#define SLED_SLED_CACHE_POLICY_LRU_CACHE_POLICY_H

#pragma once

#include "abstract_cache_policy.h"
#include "sled/log/log.h"
#include <list>
#include <map>

namespace sled {
template<typename TKey>
class LRUCachePolicy : AbstractCachePolicy<TKey> {
public:
    LRUCachePolicy(size_t size) : size_(size) { SLED_ASSERT(size > 0, "size must be greater than 0"); }

    ~LRUCachePolicy() override = default;

    void OnAdd(const TKey &key) override

    {
        keys_.push_front(key);
        key_index_[key] = keys_.begin();
    }

    void OnRemove(const TKey &key) override
    {
        auto iter = key_index_.find(key);
        if (iter != key_index_.end()) {
            keys_.erase(iter->second);
            key_index_.erase(iter);
        }
    }

    void OnGet(const TKey &key) override
    {
        auto iter = key_index_.find(key);
        if (iter != key_index_.end()) {
            keys_.erase(iter->second);
            keys_.push_front(key);
            iter->second = keys_.begin();
        }
    }

    void OnClear() override
    {
        keys_.clear();
        key_index_.clear();
    }

    void OnReplace(std::set<TKey> &elems_to_remove) override
    {
        std::size_t cur_size = key_index_.size();
        if (cur_size < size_) { return; }
        std::size_t diff = cur_size - size_;

        auto iter         = keys_.rbegin();
        std::size_t index = 0;
        while (index++ < diff) {
            elems_to_remove.insert(*iter);
            if (iter != keys_.rend()) { ++iter; }
        }
    }

    bool IsValid(const TKey &key) override { return key_index_.find(key) != key_index_.end(); }

private:
    size_t size_;
    std::list<TKey> keys_;
    std::map<TKey, typename std::list<TKey>::iterator> key_index_;
};
}// namespace sled

#endif// SLED_SLED_CACHE_POLICY_LRU_CACHE_POLICY_H
