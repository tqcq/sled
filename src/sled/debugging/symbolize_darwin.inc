#include "sled/debugging/demangle.h"
#include <assert.h>
#include <cxxabi.h>
#include <execinfo.h>

#include <algorithm>
#include <string.h>

namespace sled {
namespace {

static std::string
SLED_NO_INSTRUMENT_FUNCTION
GetSymbolString(const std::string &backtrace_line)
{
    // Example Backtrace lines:
    // 0   libimaging_shared.dylib             0x018c152a
    // _ZNSt11_Deque_baseIN3nik7mediadb4PageESaIS2_EE17_M_initialize_mapEm + 3478
    //
    // or
    // 0   libimaging_shared.dylib             0x0000000001895c39
    // _ZN3nik4util19register_shared_ptrINS_3gpu7TextureEEEvPKvS5_ + 39
    //
    // or
    // 0   mysterious_app                      0x0124000120120009 main + 17
    auto address_pos = backtrace_line.find(" 0x");
    if (address_pos == std::string::npos) return std::string();
    std::string symbol_view = backtrace_line.substr(address_pos + 1);

    auto space_pos = symbol_view.find(" ");
    if (space_pos == std::string::npos) return std::string();
    symbol_view = symbol_view.substr(space_pos + 1);// to mangled symbol

    auto plus_pos = symbol_view.find(" + ");
    if (plus_pos == std::string::npos) return std::string();
    symbol_view = symbol_view.substr(0, plus_pos);// strip remainng

    return std::string(symbol_view);
}

}// namespace

void
SLED_NO_INSTRUMENT_FUNCTION
InitializeSymbolizer(const char *argv0)
{}

bool
SLED_NO_INSTRUMENT_FUNCTION
Symbolize(const void *pc, char *out, int out_size)
{
    if (out_size <= 0 || pc == nullptr) {
        out = nullptr;
        return false;
    }

    // This allocates a char* array.
    char **frame_strings = backtrace_symbols(const_cast<void **>(&pc), 1);

    if (frame_strings == nullptr) return false;

    std::string symbol = GetSymbolString(frame_strings[0]);
    free(frame_strings);

    char tmp_buf[1024];
    if (Demangle(symbol.c_str(), tmp_buf, sizeof(tmp_buf))) {
        size_t len = strlen(tmp_buf);
        if (len + 1 <= static_cast<size_t>(out_size)) {// +1 for '\0'
            assert(len < sizeof(tmp_buf));
            memmove(out, tmp_buf, len + 1);
        }
    } else {
        strncpy(out, symbol.c_str(), static_cast<size_t>(out_size));
    }

    if (out[out_size - 1] != '\0') {
        // strncpy() does not '\0' terminate when it truncates.
        static constexpr char kEllipsis[] = "...";
        size_t ellipsis_size              = std::min(sizeof(kEllipsis) - 1, static_cast<size_t>(out_size) - 1);
        memcpy(out + out_size - ellipsis_size - 1, kEllipsis, ellipsis_size);
        out[out_size - 1] = '\0';
    }

    return true;
}
}// namespace sled
