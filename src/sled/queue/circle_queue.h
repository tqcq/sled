/**
 * @file     : circle_queue
 * @created  : 星期六  2 24, 2024 16:06:23 CST
 * @license  : MIT
 **/

#ifndef SLED_QUEUE_CIRCLE_QUEUE_H
#define SLED_QUEUE_CIRCLE_QUEUE_H
#pragma once

#include "sled/log/log.h"
#include <array>

namespace sled {

template<typename T, size_t LEN>
class CircleQueue {
    static_assert(LEN > 0, "LEN should be greater than 0.");

public:
    void Push(T &&val)
    {
        ASSERT(size() < LEN, "queue is full.");
        queue_.get(tail_) = std::move(val);
        ++tail_;
    }

    void Push(const T &val)
    {
        ASSERT(size() < LEN, "queue is full.");
        queue_.get(tail_) = val;
        ++tail_;
    }

    T &Front()
    {
        ASSERT(!empty(), "queue is empty.");
        return queue_.get(head_);
    }

    T &Back()
    {
        ASSERT(!empty(), "queue is empty.");
        return queue_.get((tail_ + LEN) % (LEN + 1));
    }

    void Pop()
    {
        ASSERT(!empty(), "queue is empty.");
        head_ = (head_ + 1) % (LEN + 1);
    }

    size_t size() const { return tail_ >= head_ ? tail_ - head_ : (LEN + 1) - (head_ - tail_); }

    bool empty() const { return (tail_ + 1) % (LEN + 1) == head_; }

    bool capacity() const { return LEN; }

private:
    std::array<T, LEN + 1> queue_;
    size_t head_ = 0;
    size_t tail_ = 0;
};

}// namespace sled

#endif// SLED_QUEUE_CIRCLE_QUEUE_H
