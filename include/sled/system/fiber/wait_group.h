#pragma once
#ifndef SLED_SYSTEM_FIBER_WAIT_GROUP_H
#define SLED_SYSTEM_FIBER_WAIT_GROUP_H
#include <marl/waitgroup.h>

namespace sled {

class WaitGroup final {
public:
    inline WaitGroup(unsigned int count = 0,
                     marl::Allocator *allocator = marl::Allocator::Default)
        : wg_(new marl::WaitGroup(count, allocator))
    {}

    inline void Add(unsigned int count = 1) const { wg_->add(count); };

    inline bool Done() const { return wg_->done(); }

    inline void Wait() const { wg_->wait(); }

private:
    mutable std::shared_ptr<marl::WaitGroup> wg_;
};
}// namespace sled

#endif// SLED_SYSTEM_FIBER_WAIT_GROUP_H
