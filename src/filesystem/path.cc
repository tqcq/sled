#include "sled/filesystem/path.h"
#include "sled/log/log.h"
#include <errno.h>
#include <unistd.h>

namespace sled {
Path
Path::Current()
{
    char tmp_path[PATH_MAX];
    if (getcwd(tmp_path, PATH_MAX)) {
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
