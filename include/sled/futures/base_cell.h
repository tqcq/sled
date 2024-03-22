#ifndef SLED_FUTURES_BASE_CELL_H
#define SLED_FUTURES_BASE_CELL_H
#include <functional>
#pragma once

namespace sled {
namespace futures {

struct BaseCell {
    void *scheduler;
};
}// namespace futures

}// namespace sled
#endif//  SLED_FUTURES_BASE_CELL_H
