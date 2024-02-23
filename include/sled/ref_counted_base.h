/**
 * @file     : ref_counted_base
 * @created  : Saturday Feb 03, 2024 12:46:20 CST
 * @license  : MIT
 **/

#ifndef REF_COUNTED_BASE_H
#define REF_COUNTED_BASE_H

#include "sled/ref_count.h"
#include "sled/ref_counter.h"
#include <type_traits>

namespace sled {

class RefCountedBase {
public:
    RefCountedBase() = default;
    RefCountedBase(const RefCountedBase &) = delete;
    RefCountedBase &operator=(const RefCountedBase &) = delete;

    void AddRef() const { ref_count_.IncRef(); }

    RefCountReleaseStatus Release() const
    {
        const auto status = ref_count_.DecRef();
        if (status == RefCountReleaseStatus::kDroppedLastRef) { delete this; }
        return status;
    }

protected:
    bool HasOneRef() const { return ref_count_.HasOneRef(); }

    virtual ~RefCountedBase() = default;

private:
    mutable sled_impl::RefCounter ref_count_{0};
};

template<typename T>
class RefCountedNonVirtual {
public:
    RefCountedNonVirtual() = default;
    RefCountedNonVirtual(const RefCountedNonVirtual &) = delete;
    RefCountedNonVirtual &operator=(const RefCountedNonVirtual &) = delete;

    void AddRef() const { ref_count_.IncRef(); }

    RefCountReleaseStatus Release() const
    {
        static_assert(!std::is_polymorphic<T>::value,
                      "T has virtual methods. RefCountedBase is a better fit.");
        const auto status = ref_count_.DecRef();
        if (status == RefCountReleaseStatus::kDroppedLastRef) {
            delete static_cast<const T *>(this);
        }
        return status;
    }

protected:
    bool HasOneRef() const { return ref_count_.HasOneRef(); }

    ~RefCountedNonVirtual() = default;

private:
    mutable sled_impl::RefCounter ref_count_{0};
};

}// namespace sled

#endif// REF_COUNTED_BASE_H
