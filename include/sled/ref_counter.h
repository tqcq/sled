/**
 * @file     : ref_counter
 * @created  : Thursday Feb 01, 2024 16:00:51 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_REF_COUNTER_H
#define SLED_REF_COUNTER_H

#include "sled/ref_count.h"
#include <atomic>

namespace sled {
namespace sled_impl {

class RefCounter {
public:
    explicit RefCounter(int ref_count) : ref_count_(ref_count) {}

    void IncRef() { ref_count_.fetch_add(1, std::memory_order_relaxed); }

    sled::RefCountReleaseStatus DecRef()
    {
        int ref_count_after_subtract =
            ref_count_.fetch_sub(1, std::memory_order_acq_rel) - 1;

        if (ref_count_after_subtract == 0) {
            return sled::RefCountReleaseStatus::kDroppedLastRef;
        }
        return sled::RefCountReleaseStatus::kOtherRefsRemained;
    }

    bool HasOneRef() const
    {
        return ref_count_.load(std::memory_order_acquire) == 1;
    }

private:
    std::atomic<int> ref_count_;
};

}// namespace sled_impl
}// namespace sled

#endif// SLED_REF_COUNTER_H
