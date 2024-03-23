/**
 * @file     : socket_factory
 * @created  : Monday Feb 12, 2024 11:36:52 CST
 * @license  : MIT
 **/

#ifndef SLED_NETWORK_SOCKET_FACTORY_H
#define SLED_NETWORK_SOCKET_FACTORY_H
#pragma once

#include "sled/network/socket.h"

namespace sled {

class SocketFactory {
public:
    virtual ~SocketFactory() = default;
    virtual Socket *CreateSocket(int family, int type) = 0;
};

}// namespace sled

#endif// SLED_NETWORK_SOCKET_FACTORY_H
