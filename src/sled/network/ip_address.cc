#include "sled/network/ip_address.h"
#include <netinet/in.h>

namespace sled {

static const in6_addr kV4MappedPrefix = {
    {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xFF, 0xFF, 0}}};
static const in6_addr kPrivateNetworkPrefix = {{{0xFD}}};

bool
IPAddress::operator==(const IPAddress &other) const
{
    if (family_ != other.family_) { return false; }

    if (family_ == AF_INET) {
        return ::memcmp(&u_.ip4, &other.u_.ip4, sizeof(u_.ip4)) == 0;
    }
    if (family_ == AF_INET6) {
        return ::memcmp(&u_.ip6, &other.u_.ip6, sizeof(u_.ip6)) == 0;
    }

    return family_ == AF_UNSPEC;
}

bool
IPAddress::operator!=(const IPAddress &other) const
{
    return !((*this) == other);
}

bool
IPAddress::operator<(const IPAddress &other) const
{
    if (family_ != other.family_) {
        if (family_ == AF_UNSPEC) { return true; }
        if (family_ == AF_INET && other.family_ == AF_INET6) { return true; }

        return false;
    }

    switch (family_) {
    case AF_INET: {
        return NetworkToHost32(u_.ip4.s_addr)
            < NetworkToHost32(other.u_.ip4.s_addr);
    }
    case AF_INET6: {
        return ::memcmp(&u_.ip6, &other.u_.ip6, sizeof(u_.ip6)) < 0;
    }
        return ::memcmp(&u_.ip6.s6_addr, &other.u_.ip6.s6_addr, 16) < 0;
    }
    return false;
}

int
IPAddress::overhead() const
{
    switch (family_) {
    case AF_INET:
        return 20;
    case AF_INET6:
        return 40;
    default:
        return 0;
    }
}

uint32_t
IPAddress::v4AddressAsHostOrderInteger() const
{
    if (family_ == AF_INET) {
        return NetworkToHost32(u_.ip4.s_addr);
    } else {
        return 0;
    }
}

bool
IPAddress::IsNil() const
{
    return IPIsUnspec(*this);
}

size_t
IPAddress::Size() const
{
    switch (family_) {
    case AF_INET:
        return sizeof(in_addr);
    case AF_INET6:
        return sizeof(in6_addr);
    default:
        return 0;
    }
}

std::string
IPAddress::ToString() const
{
    if (family_ != AF_INET && family_ != AF_INET6) { return std::string(); }

    char buf[INET6_ADDRSTRLEN] = {0};
    const void *src = &u_.ip4;
    if (family_ == AF_INET6) { src = &u_.ip6; }
    if (::inet_ntop(family_, src, buf, sizeof(buf)) == 0) {
        return std::string();
    }

    return std::string(buf);
}

in_addr
IPAddress::ipv4_address() const
{
    return u_.ip4;
}

in6_addr
IPAddress::ipv6_address() const
{
    return u_.ip6;
}

bool
IPIsHelper(const IPAddress &ip, const in6_addr &tomatch, int length)
{
    in6_addr addr = ip.ipv6_address();
    return ::memcmp(&addr, &tomatch, (length >> 3)) == 0;
}

bool
IPFromAddrInfo(struct addrinfo *info, IPAddress *out)
{
    if (!info || !info->ai_addr) { return false; }
    if (info->ai_addr->sa_family == AF_INET) {
        // ipv4
        sockaddr_in *addr = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        *out = IPAddress(addr->sin_addr);
        return true;
    } else if (info->ai_addr->sa_family == AF_INET6) {
        // ipv6
        sockaddr_in6 *addr = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        *out = IPAddress(addr->sin6_addr);
        return true;
    }
    return false;
}

bool
IPFromString(const std::string &str, IPAddress *out)
{
    if (!out) { return false; }
    in_addr addr;
    if (::inet_pton(AF_INET, str.c_str(), &addr) == 0) {
        in6_addr addr6;
        if (::inet_pton(AF_INET6, str.c_str(), &addr6) == 0) {
            *out = IPAddress();
            return false;
        }
        *out = IPAddress(addr6);
    } else {
        // RETURN VALUE is -1(invalid family) or 1(success)
        *out = IPAddress(addr);
    }
    return true;
}

bool
IPIsAny(const IPAddress &ip)
{
    switch (ip.family()) {
    case AF_INET:
        return ip == IPAddress(INADDR_ANY);
    case AF_INET6:
        return ip == IPAddress(in6addr_any) || ip == IPAddress(kV4MappedPrefix);
    case AF_UNSPEC:
        return false;
    }
    return false;
}

static bool
IPIsLinkLocalV4(const IPAddress &ip)
{
    uint32_t ip_in_host_order = ip.v4AddressAsHostOrderInteger();
    return ((ip_in_host_order >> 16) == ((169 << 8) | 254));
}

static bool
IPIsLinkLocalV6(const IPAddress &ip)
{
    in6_addr addr = ip.ipv6_address();
    return (addr.s6_addr[0] == 0xFE && (addr.s6_addr[1] & 0xC0) == 0x80);
}

bool
IPIsLinkLocal(const IPAddress &ip)
{
    switch (ip.family()) {
    case AF_INET:
        return IPIsLinkLocalV4(ip);
    case AF_INET6:
        return IPIsLinkLocalV6(ip);
    }

    return false;
}

static bool
IPIsPrivateNetworkV4(const IPAddress &ip)
{
    uint32_t ip_in_host_order = ip.v4AddressAsHostOrderInteger();
    return ((ip_in_host_order >> 24) == 10)
        || ((ip_in_host_order >> 20) == ((172 << 4) | 1))
        || ((ip_in_host_order >> 16) == ((192 << 8) | 168));
}

static bool
IPIsPrivateNetworkV6(const IPAddress &ip)
{
    return IPIsHelper(ip, kPrivateNetworkPrefix, 8);
}

// like "192.168.111.222"
bool
IPIsPrivateNetwork(const IPAddress &ip)
{
    switch (ip.family()) {
    case AF_INET:
        return IPIsPrivateNetworkV4(ip);
    case AF_INET6:
        return IPIsPrivateNetworkV6(ip);
    }
    return false;
}

static bool
IPIsSharedNetworkV4(const IPAddress &ip)
{
    uint32_t ip_in_host_order = ip.v4AddressAsHostOrderInteger();
    return (ip_in_host_order >> 22) == ((100 << 2) | 1);
}

// like "100.72.16.122"
bool
IPIsSharedNetwork(const IPAddress &ip)
{
    if (ip.family() == AF_INET) { return IPIsSharedNetworkV4(ip); }
    return false;
}

static bool
IPIsLoopbackV4(const IPAddress &ip)
{
    uint32_t ip_in_host_order = ip.v4AddressAsHostOrderInteger();
    return ((ip_in_host_order >> 24) == 127);
}

static bool
IPIsLoopbackV6(const IPAddress &ip)
{
    return ip == IPAddress(in6addr_loopback);
}

bool
IPIsLoopback(const IPAddress &ip)
{
    switch (ip.family()) {
    case AF_INET:
        return IPIsLoopbackV4(ip);
    case AF_INET6:
        return IPIsLoopbackV6(ip);
    }
    return false;
}

bool
IPIsPrivate(const IPAddress &ip)
{
    return IPIsLinkLocal(ip) || IPIsLoopback(ip) || IPIsPrivateNetwork(ip)
        || IPIsSharedNetwork(ip);
}

bool
IPIsUnspec(const IPAddress &ip)
{
    return ip.family() == AF_UNSPEC;
}

}// namespace sled
