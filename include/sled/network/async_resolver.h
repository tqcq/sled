/**
 * @file     : async_resolver
 * @created  : Monday Feb 19, 2024 18:53:03 CST
 * @license  : MIT
 **/

#pragma once
#ifndef SLED_NETWORK_ASYNC_RESOLVER_H
#define SLED_NETWORK_ASYNC_RESOLVER_H

#include "sled/network/async_resolver_interface.h"
#include "sled/scoped_refptr.h"
#include <vector>

namespace sled {

class AsyncResolver : public AsyncResolverInterface {
public:
    AsyncResolver();
    ~AsyncResolver() override;

    void Start(const SocketAddress &addr) override;
    void Start(const SocketAddress &addr, int family) override;
    bool GetResolvedAddress(int family, SocketAddress *addr) const override;
    int GetError() const override;
    void Destroy(bool wait) override;

    const std::vector<IPAddress> &addresses() const;

private:
    struct State;
    void ResolveDone(std::vector<IPAddress> addresses, int error);
    void MaybeSelfDestruct();
    SocketAddress addr_;
    std::vector<IPAddress> addresses_;
    int error_;
    bool recursion_check_ =
        false;// Protects against SignalDone calling into Destroy.
    bool destroy_called_ = false;
    scoped_refptr<State> state_;
};

}// namespace sled

#endif// SLED_NETWORK_ASYNC_RESOLVER_H
