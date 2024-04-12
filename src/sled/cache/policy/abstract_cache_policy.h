#ifndef SLED_SLED_CACHE_POLICY_ABSTRACT_CACHE_POLICY_H
#define SLED_SLED_CACHE_POLICY_ABSTRACT_CACHE_POLICY_H

#pragma once
#include <set>

namespace sled {
template<typename TKey>
class AbstractCachePolicy {
public:
    virtual ~AbstractCachePolicy() = default;

    virtual void OnUpdate(const TKey &key)
    {
        OnRemove(key);
        OnAdd(key);
    }

    virtual void OnAdd(const TKey &key)    = 0;
    virtual void OnRemove(const TKey &key) = 0;
    virtual void OnGet(const TKey &key)    = 0;
    virtual void OnClear()                 = 0;

    // 缓存策略指定需要移除哪些元素，实际移除又Cache来决定
    virtual void OnReplace(std::set<TKey> &elemes_to_remove) = 0;
    virtual bool IsValid(const TKey &key)                    = 0;
};
}// namespace sled

#endif// SLED_SLED_CACHE_POLICY_ABSTRACT_CACHE_POLICY_H
