#ifndef SLED_SLED_CACHE_POLICY_DEFAULT_CACHE_POLICY_H
#define SLED_SLED_CACHE_POLICY_DEFAULT_CACHE_POLICY_H

#pragma once

#include "abstract_cache_policy.h"

namespace sled {
template<typename TKey>
class DefaultCachePolicy : public AbstractCachePolicy<TKey> {
public:
    ~DefaultCachePolicy() override = default;

    void OnAdd(const TKey &key) override { keys_.insert(key); }

    void OnRemove(const TKey &key) override { keys_.erase(key); }

    void OnGet(const TKey &key) override {}

    void OnClear() override { keys_.clear(); }

    void OnReplace(std::set<TKey> &key) override {}

    bool IsValid(const TKey &key) override { return keys_.find(key) != keys_.end(); }

private:
    std::set<TKey> keys_;
};
}// namespace sled

#endif// SLED_SLED_CACHE_POLICY_DEFAULT_CACHE_POLICY_H
