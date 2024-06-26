#ifndef SLED_FILESYSTEM_TEMPORARY_FILE_H
#define SLED_FILESYSTEM_TEMPORARY_FILE_H
#pragma once

#include <string>

namespace sled {
class TemporaryFile {
    TemporaryFile();
    TemporaryFile(const std::string &tmp_dir);
};
}// namespace sled
#endif// SLED_FILESYSTEM_TEMPORARY_FILE_H
