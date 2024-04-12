#ifndef SLED_SLED_CACHE_LRU_CACHE_H
#define SLED_SLED_CACHE_LRU_CACHE_H

#pragma once
#include "abstract_cache.h"
#include "policy/lru_cache_policy.h"

namespace sled {
template<typename TKey, typename TValue>
class LRUCache : public AbstractCache<TKey, TValue, LRUCachePolicy<TKey>> {
public:
    LRUCache(std::size_t size) { this->policy_.insert(std::make_shared<LRUCachePolicy<TKey>>(size)); }

    ~LRUCache() override = default;
};
}// namespace sled

#endif// SLED_SLED_CACHE_LRU_CACHE_H
