#ifndef SLED_MakeUnique_H
#define SLED_MakeUnique_H
#pragma once
#include <memory>
#include <type_traits>

namespace sled {
namespace detail {
template<class T>
struct remove_extent {
    using type = T;
};

template<class T>
struct remove_extent<T[]> {
    using type = T;
};

template<class T, std::size_t N>
struct remove_extent<T[N]> {
    using type = T;
};

template<class>
struct is_unbounded_array {
    // constexpr bool is_unbounded_array_v = false;
    static constexpr bool value = false;
};

template<class T>
struct is_unbounded_array<T[]> {
    // constexpr bool is_unbounded_array_v<T[]> = true;
    static constexpr bool value = true;
};

template<class>
struct is_bounded_array {
    // constexpr bool is_bounded_array_v = false;
    static constexpr bool value = false;
};

template<class T, std::size_t N>
struct is_bounded_array<T[N]> {
    // constexpr bool is_bounded_array_v<T[N]> = true;
    static constexpr bool value = true;
};
}// namespace detail

template<class T, class... Args>
typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
MakeUnique(Args &&...args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
typename std::enable_if<detail::is_unbounded_array<T>::value, std::unique_ptr<T>>::type
MakeUnique(std::size_t n)
{
    return std::unique_ptr<T>(new typename std::remove_extent<T>::type[n]());
}

template<class T, class... Args>
typename std::enable_if<detail::is_bounded_array<T>::value>::type MakeUnique(Args &&...) = delete;

}// namespace sled
#endif// SLED_MakeUnique_H
