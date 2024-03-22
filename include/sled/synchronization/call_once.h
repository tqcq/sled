#ifndef SLED_SYNCHRONIZATION_CALL_ONCE_H
#define SLED_SYNCHRONIZATION_CALL_ONCE_H
#pragma once

#include <mutex>

namespace sled {

struct OnceFlag {
    std::once_flag flag;
};

template<typename F, typename... Args>
void
CallOnce(OnceFlag &flag, F &&f, Args &&...args)
{
    std::call_once(flag.flag, std::forward<F>(f), std::forward<Args>(args)...);
}
}// namespace sled
#endif//  SLED_SYNCHRONIZATION_CALL_ONCE_H
