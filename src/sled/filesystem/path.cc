#include "sled/filesystem/path.h"
#include "sled/log/log.h"
#include <errno.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX_SZ 4096
#else
#define PATH_MAX_SZ PATH_MAX
#endif

#if defined(_WIN32)
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif

namespace sled {
Path
Path::Current()
{
    char tmp_path[PATH_MAX_SZ];
    if (getcwd(tmp_path, PATH_MAX_SZ)) {
        auto cwd = Path(tmp_path);
        return cwd;
    }

    LOGE("Path", "getcwd failed: {}", strerror(errno));

    return Path();
}

Path
Path::Home()
{
    char *home = getenv("HOME");
    if (home) {
        return Path(home);
    } else {
        return Path();
    }
}

Path
Path::TempDir()
{
    char *tmp = getenv("TMPDIR");
    if (tmp) {
        return Path(tmp);
    } else {
        return Path("/tmp/");
    }
}

Path::Path() {}

Path::Path(const std::string &path) : path_(path) {}
}// namespace sled
