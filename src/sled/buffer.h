/**
 * @file     : buffer
 * @created  : Wednesday Feb 21, 2024 10:07:12 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_BUFFER_H
#define SLED_BUFFER_H
#include <cstring>
#include <memory>
#include <stdint.h>
#include <type_traits>

namespace sled {

namespace internal {
template<typename T, typename U>
struct BufferCompat {
    using RawU = typename std::remove_const<U>::type;

    static constexpr bool value
        = !std::is_volatile<U>::value
          && ((std::is_integral<T>::value && sizeof(T) == 1) ? (std::is_integral<U>::value && sizeof(U) == 1)
                                                             : (std::is_same<T, RawU>::value)

          );
};
}// namespace internal

template<typename T, bool ZeroOnFree = false>
class BufferT {
    static_assert(std::is_trivial<T>::value, "T must be a trivial type.");
    static_assert(!std::is_const<T>::value, "T may not be const");

public:
    using value_type     = T;
    using const_iterator = const T *;

    BufferT() : size_(0), capacity_(0), data_(nullptr) { IsConsistent(); }

    BufferT(const BufferT &)            = delete;
    BufferT &operator=(const BufferT &) = delete;

    BufferT(BufferT &&buf) : size_(buf.size()), capacity_(buf.capacity()), data_(std::move(buf.data_))
    {
        IsConsistent();
    }

    explicit BufferT(size_t size) : BufferT(size, size) {}

    BufferT(size_t size, size_t capacity)
        : size_(size),
          capacity_(std::max(size, capacity)),
          data_(capacity_ > 0 ? new T[capacity_] : nullptr)
    {
        IsConsistent();
    }

    template<typename U, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    BufferT(const U *data, size_t size) : BufferT(data, size, size)
    {}

    template<typename U, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    BufferT(U *data, size_t size, size_t capacity) : BufferT(size, capacity)
    {
        static_assert(sizeof(T) == sizeof(U), "");
        if (size > 0) { std::memcpy(data_.get(), data, size * sizeof(U)); }
    }

    template<typename U, size_t N, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    BufferT(U (&array)[N]) : BufferT(array, N)
    {}

    ~BufferT() {}

    template<typename U = T, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    const U *data() const
    {
        return reinterpret_cast<const U *>(data_.get());
    }

    template<typename U = T, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    U *data()
    {
        return reinterpret_cast<U *>(data_.get());
    }

    bool empty() const { return size_ == 0; }

    size_t size() const { return size_; }

    size_t capacity() const { return capacity_; }

    BufferT &operator=(BufferT &&buf)
    {
        size_     = buf.size_;
        capacity_ = buf.capacity_;
        using std::swap;
        swap(data_, buf.data_);
        buf.data_.reset();
        buf.OnMovedFrom();
        return *this;
    }

    bool operator==(const BufferT &buf) const
    {
        if (size_ != buf.size_) { return false; }
        if (std::is_integral<T>::value) { return std::memcmp(data_.get(), buf.data_.get(), size_ * sizeof(T)) == 0; }
        for (size_t i = 0; i < size_; i++) {
            if (data_[i] != buf.data_[i]) { return false; }
        }
        return true;
    }

    bool operator!=(const BufferT &buf) const { return !(*this == buf); }

    T &operator[](size_t index) { return data()[index]; }

    T operator[](size_t index) const { return data()[index]; }

    T *begin() { return data(); }

    T *end() { return data() + size(); }

    const T *begin() const { return data(); }

    const T *end() const { return data() + size(); }

    const T *cbegin() const { return data(); }

    const T *cend() const { return data() + size(); }

    template<typename U, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    void SetData(const U *data, size_t size)
    {
        const size_t old_size = size_;
        size_                 = 0;
        AppentData(data, size);
        if (ZeroOnFree && size_ < old_size) { ZeroTrailingData(old_size - size_); }
    }

    template<typename U, size_t N, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    void SetData(const U (&array)[N])
    {
        SetData(array, N);
    }

    template<typename U, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    void AppendData(const U *data, size_t size)
    {
        if (size == 0) { return; }

        const size_t new_size = size_ + size;
        static_assert(sizeof(T) == sizeof(U), "");
        EnsureCapacityWithHeadroom(new_size, true);
        std::memcpy(data_.get() + size_, data, size * sizeof(U));
        size_ = new_size;
    }

    template<typename U, size_t N, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    void AppendData(const U (&array)[N])
    {
        AppendData(array, N);
    }

    // template<typename W,
    //          typename std::enable_if<
    //              HasDataAndSize<const W, const T>::value>::type * = nullptr>
    // void AppendData(const W &w)
    // {
    //     AppendData(w.data(), w.size());
    // }

    template<typename U, typename std::enable_if<internal::BufferCompat<T, U>::value>::type * = nullptr>
    void AppendData(const U &item)
    {
        AppendData(&item, 1);
    }

    void SetSize(size_t size)
    {
        const size_t old_size = size_;
        EnsureCapacityWithHeadroom(size, true);
        size_ = size;
        if (ZeroOnFree && size_ < old_size) { ZeroTrailingData(old_size - size_); }
    }

    void EnsureCapacity(size_t capacity) { EnsureCapacityWithHeadroom(capacity, false); }

    void Clear() { size_ = 0; }

    friend void swap(BufferT &a, BufferT &b)
    {
        using std::swap;
        swap(a.size_, b.size_);
        swap(a.capacity_, b.capacity_);
        swap(a.data_, b.data_);
    }

private:
    void EnsureCapacityWithHeadroom(size_t capacity, bool extra_headroom)
    {
        if (capacity <= capacity_) { return; }

        const size_t new_capacity = extra_headroom ? std::max(capacity, capacity_ + capacity_ / 2) : capacity;
        std::unique_ptr<T[]> new_data(new T[new_capacity]);
        if (data_ != nullptr) { std::memcpy(new_data.get(), data_.get(), size_ * sizeof(T)); }

        data_     = std::move(new_data);
        capacity_ = new_capacity;
    }

    void ZeroTrailingData(size_t count) {}

    bool IsConsistent() const { return (data_ || capacity_ == 0) && capacity_ >= size_; }

    void OnMovedFrom()
    {
        size_     = 0;
        capacity_ = 0;
    }

    size_t size_;

    size_t capacity_;
    std::unique_ptr<T[]> data_;
};

using Buffer = BufferT<uint8_t>;
template<typename T>
using ZeroOnFreeBuffer = BufferT<T, true>;

}// namespace sled

#endif// SLED_BUFFER_H
