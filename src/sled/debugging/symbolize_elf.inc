#include <array>
#include <cxxabi.h>
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <link.h>
#include <sled/log/log.h>
#include <sled/strings/utils.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

namespace sled {

static constexpr char kTag[] = "symbolizer";
static char *g_exe           = nullptr;

static const uint64_t
GetHex(const char *start, const char *end)
{
    uint64_t hex = 0;
    for (const char *p = start; p < end; ++p) {
        int ch = *p;
        if (ch >= '0' && ch <= '9') {
            hex = hex * 16 + (ch - '0');
        } else if (ch >= 'a' && ch <= 'f') {
            hex = hex * 16 + (ch - 'a' + 10);
        } else if (ch >= 'A' && ch <= 'F') {
            hex = hex * 16 + (ch - 'A' + 10);
        } else {
            break;
        }
    }

    return hex;
}

static const uint64_t
GetHex(const std::string &str)
{
    return GetHex(str.c_str(), str.c_str() + str.size());
}

struct ObjFile {
    ObjFile() : filename(nullptr), start_addr(nullptr), end_addr(nullptr), offset(0), fd(-1), elf_type(-1)
    {
        memset(&elf_header, 0, sizeof(elf_header));
        memset(phdrs.data(), 0, sizeof(ElfW(Phdr)) * phdrs.size());
    }

    char *filename;
    const void *start_addr;
    const void *end_addr;
    uint64_t offset;

    int fd;
    int elf_type;
    ElfW(Ehdr) elf_header;

    std::array<ElfW(Phdr), 16> phdrs;
};

std::string
ReadFullFile(int fd)
{
    std::stringstream ss;
    char buf[4096];
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) { ss.write(buf, n); }
    return ss.str();
}

bool
ReadAddrMap()
{
    char maps_path[80];
    snprintf(maps_path, sizeof(maps_path), "/proc/self/task/%d/maps", getpid());
    int maps_fd;
    do {
        maps_fd = open(maps_path, O_RDONLY);
    } while (maps_fd < 0 && errno == EINTR);

    if (maps_fd < 0) {
        LOGE(kTag, "open {} failed, {}", maps_path, strerror(errno));
        return false;
    }

    std::string full_file = ReadFullFile(maps_fd);
    auto lines            = sled::StrSplit(full_file, "\n");

    // maps start_addr-end_addr permission offset dev inode pathname
    for (const auto &line : lines) {
        auto fields = sled::StrSplit(line, " \n\t", true);
        if (fields.size() < 6) { continue; }
        auto addrs = sled::StrSplit(fields[0], "-", false);

        uint64_t start_addr = GetHex(addrs[0]);
        uint64_t end_addr   = GetHex(addrs[1]);
        uint64_t offset     = GetHex(fields[2]);
        LOGD(kTag, "addr: {}-{} {} {} {}", start_addr, end_addr, offset, fields[1], fields[5]);
    }

    return true;
}

class Symbolizer {};

void
InitializeSymbolizer(const char *argv0)
{
    if (g_exe) {
        free(g_exe);
        g_exe = nullptr;
    }

    if (argv0 && argv0[0] != '\0') { g_exe = strdup(argv0); }
}

bool
Symbolize(const void *pc, char *out, int out_size)
{
    Dl_info info;
    if (!dladdr(pc, &info)) { return false; }
    if (info.dli_fname && out_size > 0) {
        strncpy(out, info.dli_fname, out_size);

        const auto end_pos = static_cast<size_t>(out_size) - 1;
        if (out[end_pos] != '\0') {
            // add ...
            out[end_pos] = '\0';
        }
    }

    return true;
}
}// namespace sled
