/**
 * @file     : socket_address
 * @created  : Monday Feb 19, 2024 14:36:57 CST
 * @license  : MIT
 **/

#ifndef SOCKET_ADDRESS_H
#define SOCKET_ADDRESS_H

#include "sled/network/ip_address.h"

namespace sled {

class SocketAddress {
public:
    SocketAddress();
    SocketAddress(const std::string &hostname, int port);
    SocketAddress(uint32_t ip_as_host_order_integer, int port);
    SocketAddress(const IPAddress &ip, int port);
    SocketAddress(const SocketAddress &addr);

    // Resets to the nil address
    void Clear();
    // empty hostname, any IP, null port
    bool IsNil() const;
    // Returns true if ip and port are set
    bool IsComplete() const;
    SocketAddress &operator=(const SocketAddress &addr);

    // set ip
    void SetIP(uint32_t ip_as_host_order_integer);
    void SetIP(const IPAddress &ip);
    void SetIP(const std::string &hostname);
    void SetResolvedIP(uint32_t ip_as_host_order_integer);
    void SetResolvedIP(const IPAddress &ip);
    void SetPort(int port);

    const std::string &hostname() const { return hostname_; }

    uint32_t ip() const;

    int family() const { return ip_.family(); }

    const IPAddress &ipaddr() const;
    uint16_t port() const;

    int scope_id() const { return scope_id_; }

    void SetScopeID(int id) { scope_id_ = id; }

    bool IsAnyIP() const;
    bool IsLoopbackIP() const;
    bool IsPrivateIP() const;
    bool IsUnresolvedIP() const;
    size_t ToSockAddrStorage(sockaddr_storage *saddr) const;
    void ToSockAddr(sockaddr_in *saddr) const;
    bool FromSockAddr(const sockaddr_in &saddr);

private:
    std::string hostname_;
    IPAddress ip_;
    uint16_t port_;
    int scope_id_;
    bool literal_;
};

bool SocketAddressFromSockAddrStorage(const sockaddr_storage &saddr,
                                      SocketAddress *out);
}// namespace sled

#endif// SOCKET_ADDRESS_H
