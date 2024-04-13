#pragma once
#ifndef SLED_SYSTEM_PID_H
#define SLED_SYSTEM_PID_H

#ifdef _WIN32
#include <Windows.h>
#define getpid (int) GetCurrentProcessId
typedef unsigned long pid_t;
#else
#include <unistd.h>
#endif

namespace sled {
#ifdef _WIN32
typedef unsigned long ProcessId;
#else
typedef pid_t ProcessId;
#endif

ProcessId GetCachedPID();
}// namespace sled
#endif// SLED_SYSTEM_PID_H
