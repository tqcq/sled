#ifndef SLED_SLED_CACHE_EXPIRE_CACHE_H
#define SLED_SLED_CACHE_EXPIRE_CACHE_H

#pragma once
#include "abstract_cache.h"
#include "policy/expire_cache_policy.h"

namespace sled {
template<typename TKey, typename TValue>
class ExpireCache : public AbstractCache<TKey, TValue, ExpireCachePolicy<TKey>> {
public:
    ExpireCache(const TimeDelta &expire_time)
    {
        this->policy_.insert(std::make_shared<ExpireCachePolicy<TKey>>(expire_time));
    }

    ~ExpireCache() override = default;
};
}// namespace sled

#endif// SLED_SLED_CACHE_EXPIRE_CACHE_H
