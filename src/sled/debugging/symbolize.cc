#include "sled/debugging/symbolize.h"
#include "sled/lang/attributes.h"

#ifdef _WIN32
namespace sled {
void
InitializeSymbolizer(const char *argv0)
{}

bool
Symbolize(const void *pc, char *out, int out_size)
{
    return false;
}
}// namespace sled
#elif defined(__APPLE__)
#include "symbolize_darwin.inc"
#elif defined(__linux__)
namespace sled {
void
InitializeSymbolizer(const char *argv0)
{}

bool
Symbolize(const void *pc, char *out, int out_size)
{
    return false;
}
}// namespace sled

// #include "sled/debugging/symbolize_elf.inc"
#endif
