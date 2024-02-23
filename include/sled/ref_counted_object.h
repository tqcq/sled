/**
 * @file     : ref_counted_object
 * @created  : Thursday Feb 01, 2024 16:09:01 CST
 * @license  : MIT
 **/

#ifndef REF_COUNTED_OBJECT_H
#define REF_COUNTED_OBJECT_H

#include "sled/ref_count.h"
#include "sled/ref_counter.h"

namespace sled {

template<typename T>
class RefCountedObject : public T {
public:
    RefCountedObject() = default;

    RefCountedObject(const RefCountedObject &) = delete;
    RefCountedObject operator=(const RefCountedObject &) = delete;

    template<typename TObject>
    explicit RefCountedObject(TObject &&obj) : T(std::forward<TObject>(obj))
    {}

    void AddRef() const override { ref_count_.IncRef(); }

    RefCountReleaseStatus Release() const override
    {
        const auto status = ref_count_.DecRef();
        if (status == RefCountReleaseStatus::kDroppedLastRef) { delete this; }
        return status;
    }

    virtual bool HasOneRef() const { return ref_count_.HasOneRef(); }

protected:
    ~RefCountedObject() override {}

    mutable sled_impl::RefCounter ref_count_{0};
};

template<typename T>
class FinalRefCountedObject final : public T {
public:
    using T::T;

    explicit FinalRefCountedObject(T &&other) : T(std::move(other)) {}

    FinalRefCountedObject(const FinalRefCountedObject &) = delete;
    FinalRefCountedObject operator=(const FinalRefCountedObject &) = delete;

    void AddRef() const { ref_count_.IncRef(); }

    RefCountReleaseStatus Release() const
    {
        const auto status = ref_count_.DecRef();
        if (status == RefCountReleaseStatus::kDroppedLastRef) { delete this; }
        return status;
    }

    bool HasOneRef() const { return ref_count_.HasOneRef(); }

private:
    ~FinalRefCountedObject() = default;
    mutable sled_impl::RefCounter ref_count_{0};
};

}// namespace sled

#endif// REF_COUNTED_OBJECT_H
