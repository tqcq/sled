#ifndef SLED_META_TYPE_TRAITS_H
#define SLED_META_TYPE_TRAITS_H

#pragma once

#include "sled/exec/detail/invoke_result.h"

namespace sled {

template<typename Callable, typename... Args>
using InvokeResultT = typename eggs::invoke_result_t<Callable, Args...>;

template<typename Callable, typename... Args>
using IsInvocable = eggs::is_invocable<Callable, Args...>;

template<typename Convertible, typename Callable, typename... Args>
using IsInvocableR = eggs::is_invocable_r<Convertible, Callable, Args...>;

template<typename Convertible, typename Callable, typename... Args>
using IsNothrowInvocable = eggs::is_nothrow_invocable<Convertible, Callable, Args...>;

template<typename Convertible, typename Callable, typename... Args>
using IsNothrowInvocableR = eggs::is_nothrow_invocable_r<Convertible, Callable, Args...>;

// EnableIfT
template<bool B, typename T = void>
using EnableIfT = typename std::enable_if<B, T>::type;

template<typename T>
using DecayT = typename std::decay<T>::type;

}// namespace sled

#endif// SLED_META_TYPE_TRAITS_H
