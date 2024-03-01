/**
 * @file     : location
 * @created  : Sunday Feb 04, 2024 20:58:26 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_SYSTEM_LOCATION_H
#define SLED_SYSTEM_LOCATION_H

#include <string>

namespace sled {

class Location final {
public:
    Location() = delete;

    static Location Current(const char *file_name = __builtin_FILE(),
                            int file_line = __builtin_LINE(),
                            const char *function = __builtin_FUNCTION());

    std::string ToString() const;

private:
    Location(const char *file_name, int file_line, const char *function);

    const char *file_name;
    int file_line;
    const char *function;
};

}// namespace sled

#define SLED_FROM_HERE sled::Location::Current();

#endif// SLED_SYSTEM_LOCATION_H
