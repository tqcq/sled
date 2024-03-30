#include "sled/debugging/symbolize.h"

#ifdef _WIN32
void
InitializeSymbolizer(const char *argv0)
{}

bool
Symbolize(const void *pc, char *out, int out_size)
{
    return false;
}
#elif defined(__APPLE__)
void
InitializeSymbolizer(const char *argv0)
{}

bool
Symbolize(const void *pc, char *out, int out_size)
{
    return false;
}
#elif defined(__linux__)
#include "sled/debugging/symbolize_elf.inc"
#endif
