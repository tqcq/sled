/**
 * @file     : ip_address
 * @created  : Monday Feb 19, 2024 14:38:40 CST
 * @license  : MIT
 **/

#ifndef SLED_NETWORK_IP_ADDRESS_H
#define SLED_NETWORK_IP_ADDRESS_H
#pragma once

#include "sled/byte_order.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>

namespace sled {

class IPAddress {
public:
    IPAddress() : family_(AF_UNSPEC) { ::memset(&u_, 0, sizeof(u_)); }

    explicit IPAddress(const in_addr &ip4) : family_(AF_INET)
    {
        ::memset(&u_, 0, sizeof(u_));
        u_.ip4 = ip4;
    }

    explicit IPAddress(const in6_addr &ip6) : family_(AF_INET6) { u_.ip6 = ip6; }

    explicit IPAddress(uint32_t ip_in_host_byte_order) : family_(AF_INET)
    {
        ::memset(&u_, 0, sizeof(u_));
        u_.ip4.s_addr = HostToNetwork32(ip_in_host_byte_order);
    }

    IPAddress(const IPAddress &other) : family_(other.family_) { ::memcpy(&u_, &other.u_, sizeof(u_)); }

    virtual ~IPAddress() = default;

    const IPAddress &operator=(const IPAddress &other)
    {
        family_ = other.family_;
        ::memcpy(&u_, &other.u_, sizeof(u_));
        return *this;
    }

    bool operator==(const IPAddress &other) const;
    bool operator!=(const IPAddress &other) const;
    bool operator<(const IPAddress &other) const;
    bool operator>(const IPAddress &other) const;

    int family() const { return family_; };

    in_addr ipv4_address() const;
    in6_addr ipv6_address() const;
    size_t Size() const;
    std::string ToString() const;
    IPAddress Normailzed() const;
    IPAddress AsIPv6Address() const;
    uint32_t v4AddressAsHostOrderInteger() const;
    int overhead() const;
    bool IsNil() const;

private:
    int family_;

    union {
        in_addr ip4;
        in6_addr ip6;
    } u_;
};

bool IPFromAddrInfo(struct addrinfo *info, IPAddress *out);

bool IPFromString(const std::string &str, IPAddress *out);

bool IPIsAny(const IPAddress &ip);
bool IPIsLoopback(const IPAddress &ip);
bool IPIsLinkLocal(const IPAddress &ip);
// like "192.168.111.222"
bool IPIsPrivateNetwork(const IPAddress &ip);
// like "100.72.16.122"
bool IPIsSharedNetwork(const IPAddress &ip);

bool IPIsPrivate(const IPAddress &ip);
bool IPIsUnspec(const IPAddress &ip);

}// namespace sled

#endif// SLED_NETWORK_IP_ADDRESS_H
