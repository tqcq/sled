/**
 * @file     : scoped_refptr
 * @created  : Thursday Feb 01, 2024 16:24:37 CST
 * @license  : MIT
 **/

#ifndef SCOPED_REFPTR_H
#define SCOPED_REFPTR_H

#include <memory>

namespace sled {

template<typename T>
class scoped_refptr {
public:
    using element_type = T;

    scoped_refptr() : ptr_(nullptr) {}

    scoped_refptr(std::nullptr_t) : ptr_(nullptr) {}

    explicit scoped_refptr(T *p) : ptr_(p)
    {
        if (ptr_) ptr_->AddRef();
    }

    scoped_refptr(const scoped_refptr &r) : ptr_(r.ptr_)
    {
        if (ptr_) ptr_->AddRef();
    }

    template<typename U>
    scoped_refptr(const scoped_refptr<U> &r) : ptr_(r.get())
    {
        if (ptr_) ptr_->AddRef();
    }

    scoped_refptr(scoped_refptr<T> &&r) noexcept : ptr_(r.release()) {}

    template<typename U>
    scoped_refptr(scoped_refptr<U> &&r) noexcept : ptr_(r.release())
    {}

    ~scoped_refptr()
    {
        if (ptr_) ptr_->Release();
    }

    T *get() const { return ptr_; }

    explicit operator bool() { return ptr_ != nullptr; }

    T &operator*() const { return *ptr_; }

    T *operator->() const { return ptr_; }

    T *release()
    {
        T *ret_val = ptr_;
        ptr_ = nullptr;
        return ret_val;
    }

    scoped_refptr<T> &operator=(T *p)
    {
        if (p) p->AddRef();
        if (ptr_) ptr_->Release();
        ptr_ = p;
        return *this;
    }

    scoped_refptr<T> &operator=(const scoped_refptr<T> &r)
    {
        return *this = r.get();
    }

    template<typename U>
    scoped_refptr<T> &operator=(const scoped_refptr<U> &r)
    {
        return *this = r.get();
    }

    scoped_refptr<T> &operator=(const scoped_refptr<T> &&r) noexcept
    {
        scoped_refptr<T>(std::move(r)).swap(*this);
    }

    template<typename U>
    scoped_refptr<T> &operator=(const scoped_refptr<U> &&r)
    {
        scoped_refptr<T>(std::move(r)).swap(*this);
        return *this;
    }

    void swap(T **pp) noexcept
    {
        T *tmp = ptr_;
        ptr_ = *pp;
        *pp = tmp;
    }

    void swap(scoped_refptr<T> &r) noexcept { swap(&r.ptr_); }

protected:
    T *ptr_;
};

template<typename T, typename U>
bool
operator==(const scoped_refptr<T> &lhs, const scoped_refptr<U> &rhs)
{
    return lhs.get() == rhs.get();
}

template<typename T, typename U>
bool
operator!=(const scoped_refptr<T> &lhs, const scoped_refptr<U> &rhs)
{
    return !(lhs == rhs);
}

template<typename T>
bool
operator==(const scoped_refptr<T> &lhs, std::nullptr_t)
{
    return lhs.get() == nullptr;
}

template<typename T>
bool
operator!=(const scoped_refptr<T> &lhs, std::nullptr_t)
{
    return !(lhs == nullptr);
}

template<typename T>
bool
operator==(std::nullptr_t, const scoped_refptr<T> &rhs)
{
    return rhs.get() == nullptr;
}

template<typename T>
bool
operator!=(std::nullptr_t, const scoped_refptr<T> &rhs)
{
    return !(nullptr == rhs);
}

template<typename T, typename U>
bool
operator==(const scoped_refptr<T> &lhs, const U *rhs)
{
    return lhs.get() == rhs;
}

template<typename T, typename U>
bool
operator!=(const scoped_refptr<T> &lhs, const U *rhs)
{
    return !(lhs == rhs);
}

template<typename T, typename U>
bool
operator==(const T *lhs, const scoped_refptr<U> &rhs)
{
    return lhs == rhs.get();
}

template<typename T, typename U>
bool
operator!=(const T *lhs, const scoped_refptr<U> &rhs)
{
    return !(lhs == rhs);
}

// for std::map
template<typename T, typename U>
bool
operator<(const scoped_refptr<T> &lhs, const scoped_refptr<U> &rhs)
{
    return lhs.get() < rhs.get();
}

}// namespace sled

#endif// SCOPED_REFPTR_H
