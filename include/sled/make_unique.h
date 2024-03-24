#ifndef SLED_MAKE_UNIQUE_H
#define SLED_MAKE_UNIQUE_H
#pragma once
#include <memory>
#include <type_traits>

namespace sled {
template<typename T, typename... Args>
inline auto
MakeUnique(Args &&...args) -> std::unique_ptr<T>
{
    return std::move(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

template<typename T, typename U = T>
inline typename std::enable_if<!std::is_pointer<T>::value && !std::is_pointer<U>::value && std::is_base_of<T, U>::value,
                               std::unique_ptr<T>>::type
MakeUnique(U *ptr)
{
    return std::move(std::unique_ptr<T>(ptr));
}

}// namespace sled
#endif// SLED_MAKE_UNIQUE_H
