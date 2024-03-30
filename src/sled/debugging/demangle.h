#ifndef SLED_DEBUGGING_DEMANGLE_H
#define SLED_DEBUGGING_DEMANGLE_H
#pragma once

#include <string>

namespace sled {
bool Demangle(const char *mangled, char *out, size_t out_size);
std::string DemangleString(const char *mangled);
}// namespace sled
#endif// SLED_DEBUGGING_DEMANGLE_H
