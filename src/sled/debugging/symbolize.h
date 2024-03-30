#ifndef SLED_DEBUGGING_SYMBOLIZE_H
#define SLED_DEBUGGING_SYMBOLIZE_H
#pragma once

namespace sled {
void InitializeSymbolizer(const char *argv0);
bool Symbolize(const void *pc, char *out, int out_size);
bool ReadAddrMap();
}// namespace sled
#endif// SLED_DEBUGGING_SYMBOLIZE_H
