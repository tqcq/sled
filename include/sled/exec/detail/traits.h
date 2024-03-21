#ifndef SLED_EXEC_DETAIL_TRAITS_H
#define SLED_EXEC_DETAIL_TRAITS_H
#pragma once
#include "invoke_result.h"
#include <type_traits>

namespace sled {
template<typename TSender, typename TReceiver>
using ConnectResultT = decltype(std::declval<TSender>().Connect(std::declval<TReceiver>()));

template<typename TSender>
using SenderResultT = typename TSender::result_t;

}// namespace sled

#endif// SLED_EXEC_DETAIL_TRAITS_H
