#pragma once
#ifndef SLED_FILESYSTEM_PATH_H
#define SLED_FILESYSTEM_PATH_H

#include <string>

namespace sled {
class Path {
public:
    // cwd = current working directory
    static Path Current();
    static Path Home();
    static Path TempDir();

    Path();
    Path(const std::string &path);

    std::string ToString() const { return path_; };

private:
    std::string path_;
};
}// namespace sled
#endif// SLED_FILESYSTEM_PATH_H
