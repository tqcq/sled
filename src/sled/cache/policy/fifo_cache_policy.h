#ifndef SLED_SLED_CACHE_POLICY_FIFO_CACHE_POLICY_H
#define SLED_SLED_CACHE_POLICY_FIFO_CACHE_POLICY_H

#pragma once

#include "default_cache_policy.h"
#include "sled/log/log.h"
#include <list>

namespace sled {
template<typename TKey>
class FIFOCachePolicy : public DefaultCachePolicy<TKey> {
private:
    using BaseClass = DefaultCachePolicy<TKey>;

public:
    FIFOCachePolicy(size_t size) : size_(size) { SLED_ASSERT(size > 0, "size must be greater than 0"); }

    ~FIFOCachePolicy() override = default;

    void OnAdd(const TKey &key) override

    {
        BaseClass::OnAdd(key);
        keys_.push_back(key);
    }

    void OnRemove(const TKey &key) override
    {
        if (BaseClass::IsValid(key)) {
            keys_.remove(key);
            BaseClass::OnRemove(key);
        }
    }

    void OnGet(const TKey &key) override {}

    void OnClear() override
    {
        keys_.clear();
        BaseClass::OnClear();
    }

    void OnReplace(std::set<TKey> &elems_to_remove) override
    {
        BaseClass::OnReplace(elems_to_remove);

        if (keys_.size() <= size_) { return; }
        std::size_t diff  = keys_.size() - size_;
        std::size_t index = 0;
        auto iter         = keys_.begin();
        while (index++ < diff) {
            elems_to_remove.insert(*iter);
            if (iter != keys_.end()) { ++iter; }
        }
    }

    bool IsValid(const TKey &key) override { return BaseClass::IsValid(key); }

private:
    size_t size_;
    std::list<TKey> keys_;
};
}// namespace sled

#endif// SLED_SLED_CACHE_POLICY_FIFO_CACHE_POLICY_H
