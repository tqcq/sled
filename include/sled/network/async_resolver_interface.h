/**
 * @file     : async_resolver_interface
 * @created  : Monday Feb 19, 2024 18:50:17 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_NETWORK_ASYNC_RESOLVER_INTERFACE_H
#define SLED_NETWORK_ASYNC_RESOLVER_INTERFACE_H

#include "sled/network/socket_address.h"
#include "sled/sigslot.h"

namespace sled {

class AsyncResolverInterface {
public:
    AsyncResolverInterface() = default;
    virtual ~AsyncResolverInterface() = default;

    virtual void Start(const SocketAddress &addr) = 0;
    virtual void Start(const SocketAddress &addr, int family) = 0;
    virtual bool GetResolvedAddress(int family, SocketAddress *addr) const = 0;
    virtual int GetError() const = 0;
    virtual void Destroy(bool wait) = 0;

    inline SocketAddress address() const
    {
        SocketAddress addr;
        GetResolvedAddress(AF_INET, &addr);
        return addr;
    }

    sigslot::signal1<AsyncResolverInterface *> SignalDone;
};

}// namespace sled

#endif// SLED_NETWORK_ASYNC_RESOLVER_INTERFACE_H
