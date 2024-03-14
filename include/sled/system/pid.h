#pragma once
#ifndef SLED_SYSTEM_PID_H
#define SLED_SYSTEM_PID_H

#include <unistd.h>

namespace sled {
pid_t GetCachedPID();
}// namespace sled
#endif// SLED_SYSTEM_PID_H
