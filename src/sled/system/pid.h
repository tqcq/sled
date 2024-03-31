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
pid_t GetCachedPID();
}// namespace sled
#endif// SLED_SYSTEM_PID_H
