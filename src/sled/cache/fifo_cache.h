#ifndef SLED_SLED_CACHE_FIFO_CACHE_H
#define SLED_SLED_CACHE_FIFO_CACHE_H

#pragma once
#include "abstract_cache.h"
#include "policy/fifo_cache_policy.h"

namespace sled {
template<typename TKey, typename TValue>
class FIFOCache : public AbstractCache<TKey, TValue, FIFOCachePolicy<TKey>> {
public:
    FIFOCache(std::size_t size) { this->policy_.insert(std::make_shared<FIFOCachePolicy<TKey>>(size)); }

    ~FIFOCache() override = default;
};
}// namespace sled

#endif// SLED_SLED_CACHE_FIFO_CACHE_H
