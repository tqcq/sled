#ifndef SLED_MAKE_UNIQUE_H
#define SLED_MAKE_UNIQUE_H
#pragma once
#include <memory>

namespace sled {
template<typename T, typename... Args>
inline auto
MakeUnique(Args &&...args) -> std::unique_ptr<T>
{
    return std::move(std::unique_ptr<T>(new T(std::forward<Args>(args)...)));
}

}// namespace sled
#endif// SLED_MAKE_UNIQUE_H
