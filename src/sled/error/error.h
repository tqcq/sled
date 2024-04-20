#ifndef SLED_ERROR_ERROR_H
#define SLED_ERROR_ERROR_H

#pragma once

namespace sled {
class Error {
public:
    template<typename T>
    static Error Wrap();
};
}// namespace sled

#endif// SLED_ERROR_ERROR_H
