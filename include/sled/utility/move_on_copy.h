#ifndef SLED_UTILITY_MOVE_ON_COPY_H
#define SLED_UTILITY_MOVE_ON_COPY_H
#include <memory>
#include <utility>

namespace sled {

template<typename T>
struct MoveOnCopy {
    using type = typename std::remove_reference<T>::type;

    MoveOnCopy(type &&value) : value(std::move(value)) {}

    MoveOnCopy(const MoveOnCopy &other) : value(std::move(other.value)) {}

    MoveOnCopy(MoveOnCopy &&other) : value(std::move(other.value)) {}

    // MoveOnCopy(MoveOnCopy &&)                 = delete;
    MoveOnCopy &operator=(const MoveOnCopy &) = delete;

    mutable type value;
};

template<typename T>
auto
MakeMoveOnCopy(T &&value) -> MoveOnCopy<T>
{
    return {std::move<T>(value)};
}
}// namespace sled
#endif//  SLED_UTILITY_MOVE_ON_COPY_H
