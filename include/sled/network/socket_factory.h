/**
 * @file     : socket_factory
 * @created  : Monday Feb 12, 2024 11:36:52 CST
 * @license  : MIT
 **/

#ifndef SOCKET_FACTORY_H
#define SOCKET_FACTORY_H

#include "sled/network/socket.h"

namespace sled {

class SocketFactory {
public:
    virtual ~SocketFactory() = default;
    virtual Socket *CreateSocket(int family, int type) = 0;
};

}// namespace sled

#endif// SOCKET_FACTORY_H
