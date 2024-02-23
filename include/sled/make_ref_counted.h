/**
 * @file     : make_ref_counted
 * @created  : Thursday Feb 01, 2024 16:18:30 CST
 * @license  : MIT
 **/

#ifndef MAKE_REF_COUNTED_H
#define MAKE_REF_COUNTED_H

#include "sled/ref_count.h"
#include "sled/ref_counted_object.h"
#include "sled/scoped_refptr.h"
#include <type_traits>
#include <utility>

namespace sled {

namespace internal {
template<typename T>
class HasAddRefAndRelease {
private:
    template<typename TClass,
             decltype(std::declval<TClass>().AddRef()) * = nullptr,
             decltype(std::declval<TClass>().Release()) * = nullptr>
    static int Test(int);

    template<typename>
    static char Test(...);

public:
    static constexpr bool value =
        std::is_same<decltype(Test<T>(0)), int>::value;
};
}// namespace internal

template<
    typename T,
    typename... Args,
    typename std::enable_if<std::is_convertible<T *, RefCountInterface *>::value
                                && std::is_abstract<T>::value,
                            T>::type * = nullptr>
scoped_refptr<T>
make_ref_counted(Args &&...args)
{

    return scoped_refptr<T>(
        new RefCountedObject<T>(std::forward<Args>(args)...));
}

template<typename T,
         typename... Args,
         typename std::enable_if<
             !std::is_convertible<T *, RefCountInterface *>::value
                 && internal::HasAddRefAndRelease<T>::value,
             T>::type * = nullptr>
scoped_refptr<T>
make_ref_counted(Args &&...args)
{
    return scoped_refptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T,
         typename... Args,
         typename std::enable_if<
             !std::is_convertible<T *, RefCountInterface *>::value
             && !internal::HasAddRefAndRelease<T>::value>::type * = nullptr>
scoped_refptr<FinalRefCountedObject<T>>
make_ref_counted(Args &&...args)
{
    return scoped_refptr<FinalRefCountedObject<T>>(
        new FinalRefCountedObject<T>(std::forward<Args>(args)...));
}

}// namespace sled

#endif// MAKE_REF_COUNTED_H
