#ifndef SLED_DEBUGGING_DEMANGLE_H
#define SLED_DEBUGGING_DEMANGLE_H
#pragma once

#include "sled/lang/attributes.h"
#include <string>

namespace sled {
bool SLED_NO_INSTRUMENT_FUNCTION Demangle(const char *mangled, char *out, size_t out_size);
std::string SLED_NO_INSTRUMENT_FUNCTION DemangleString(const char *mangled);
}// namespace sled
#endif// SLED_DEBUGGING_DEMANGLE_H
