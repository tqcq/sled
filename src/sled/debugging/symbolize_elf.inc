#include "sled/log/log.h"
#include <dwarf++.hh>
#include <elf++.hh>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

namespace sled {

static elf::elf *g_elf       = nullptr;
static dwarf::dwarf *g_dwarf = nullptr;

void
InitializeSymbolizer(const char *argv0)
{
    if (g_elf) { return; }
    int self_fd = open(argv0, O_RDONLY);
    if (self_fd < 0) { return; }
    g_elf   = new elf::elf(elf::create_mmap_loader(self_fd));
    g_dwarf = new dwarf::dwarf(dwarf::elf::create_loader(*g_elf));
}

static bool
FindPC(dwarf::taddr pc)
{
    return false;
}

bool
Symbolize(const void *pc, char *out, int out_size)
{
    if (!g_elf || !g_dwarf) { return false; }
    if (out_size <= 0) { return false; }
    out[0] = '\0';

    union {
        const void *void_pc;
        dwarf::taddr pc_addr;
    } u;

    u.void_pc  = pc;
    bool found = false;
    for (auto &cu : g_dwarf->compilation_units()) {
        try {
            if (dwarf::die_pc_range(cu.root()).contains(u.pc_addr)) {
                auto &lt  = cu.get_line_table();
                auto iter = lt.find_address(u.pc_addr);
                if (iter == lt.end()) {
                    return false;
                } else {
                    LOGD("found line {}", iter->get_description());
                }
                found = true;
                break;
            }
        } catch (...) {}
    }

    return found;
}
}// namespace sled
