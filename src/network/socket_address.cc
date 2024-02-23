#include "sled/network/socket_address.h"
#include "sled/network/ip_address.h"

namespace sled {

SocketAddress::SocketAddress() { Clear(); }

SocketAddress::SocketAddress(const std::string &hostname, int port)
{
    SetIP(hostname);
    SetPort(port);
}

SocketAddress::SocketAddress(uint32_t ip_as_host_order_integer, int port)
{
    SetIP(IPAddress(ip_as_host_order_integer));
    SetPort(port);
}

SocketAddress::SocketAddress(const IPAddress &ip, int port)
{
    SetIP(ip);
    SetPort(port);
}

SocketAddress::SocketAddress(const SocketAddress &addr)
{
    this->operator=(addr);
}

void
SocketAddress::Clear()
{
    hostname_.clear();
    literal_ = false;
    ip_ = IPAddress();
    port_ = 0;
    scope_id_ = 0;
}

bool
SocketAddress::IsNil() const
{
    return hostname_.empty() && IPIsUnspec(ip_) && 0 == port_;
}

bool
SocketAddress::IsComplete() const
{
    return (!IPIsAny(ip_)) && (0 != port_);
}

SocketAddress &
SocketAddress::operator=(const SocketAddress &addr)
{
    hostname_ = addr.hostname_;
    ip_ = addr.ip_;
    port_ = addr.port_;
    scope_id_ = addr.scope_id_;
    literal_ = addr.literal_;
    return *this;
}

void
SocketAddress::SetIP(uint32_t ip_as_host_order_integer)
{
    hostname_.clear();
    literal_ = false;
    ip_ = IPAddress(ip_as_host_order_integer);
    scope_id_ = 0;
}

void
SocketAddress::SetIP(const IPAddress &ip)
{
    hostname_.clear();
    literal_ = false;
    ip_ = ip;
    scope_id_ = 0;
}

void
SocketAddress::SetIP(const std::string &hostname)
{
    hostname_ = hostname;
    literal_ = IPFromString(hostname, &ip_);
    if (!literal_) { ip_ = IPAddress(); }
    scope_id_ = 0;
}

void
SocketAddress::SetResolvedIP(uint32_t ip_as_host_order_integer)
{
    SetIP(ip_as_host_order_integer);
    scope_id_ = 0;
}

void
SocketAddress::SetResolvedIP(const IPAddress &ip)
{
    ip_ = ip;
    scope_id_ = 0;
}

void
SocketAddress::SetPort(int port)
{
    port_ = static_cast<uint16_t>(port);
}

uint32_t
SocketAddress::ip() const
{
    return ip_.v4AddressAsHostOrderInteger();
}

const IPAddress &
SocketAddress::ipaddr() const
{
    return ip_;
}

uint16_t
SocketAddress::port() const
{
    return port_;
}

bool
SocketAddress::IsAnyIP() const
{
    return IPIsAny(ip_);
}

bool
SocketAddress::IsLoopbackIP() const
{
    return IPIsLoopback(ip_)
        || (IPIsAny(ip_) && 0 == strcmp(hostname_.c_str(), "localhost"));
}

bool
SocketAddress::IsPrivateIP() const
{

    return IPIsPrivate(ip_);
}

bool
SocketAddress::IsUnresolvedIP() const
{
    return IPIsUnspec(ip_) && !literal_ && !hostname_.empty();
}

void
SocketAddress::ToSockAddr(sockaddr_in *saddr) const
{
    ::memset(saddr, 0, sizeof(*saddr));
    if (ip_.family() != AF_INET) {
        saddr->sin_family = AF_UNSPEC;
        return;
    }

    saddr->sin_family = AF_INET;
    saddr->sin_port = HostToNetwork16(port_);
    if (IPIsAny(ip_)) {
        saddr->sin_addr.s_addr = INADDR_ANY;
    } else {
        saddr->sin_addr = ip_.ipv4_address();
    }
}

bool
SocketAddress::FromSockAddr(const sockaddr_in &saddr)
{
    if (saddr.sin_family != AF_INET) { return false; }

    SetIP(NetworkToHost32(saddr.sin_addr.s_addr));
    SetPort(NetworkToHost16(saddr.sin_port));
    literal_ = false;
    return true;
}

static size_t
ToSocketAddrStorageHelper(sockaddr_storage *addr,
                          const IPAddress &ip,
                          uint16_t port,
                          int scope_id)
{
    ::memset(addr, 0, sizeof(sockaddr_storage));
    addr->ss_family = static_cast<sa_family_t>(ip.family());
    if (addr->ss_family == AF_INET6) {
        sockaddr_in6 *saddr = reinterpret_cast<sockaddr_in6 *>(addr);
        saddr->sin6_addr = ip.ipv6_address();
        saddr->sin6_port = HostToNetwork16(port);
        saddr->sin6_scope_id = scope_id;
        return sizeof(sockaddr_in6);
    } else if (addr->ss_family == AF_INET) {
        sockaddr_in *saddr = reinterpret_cast<sockaddr_in *>(addr);
        saddr->sin_addr = ip.ipv4_address();
        saddr->sin_port = HostToNetwork16(port);
        return sizeof(sockaddr_in);
    }
    return 0;
}

size_t
SocketAddress::ToSockAddrStorage(sockaddr_storage *saddr) const
{
    return ToSocketAddrStorageHelper(saddr, ip_, port_, scope_id_);
}

bool
SocketAddressFromSockAddrStorage(const sockaddr_storage &addr,
                                 SocketAddress *out)
{
    if (!out) { return false; }

    if (addr.ss_family == AF_INET) {
        const sockaddr_in *saddr = reinterpret_cast<const sockaddr_in *>(&addr);
        *out = SocketAddress(IPAddress(saddr->sin_addr),
                             NetworkToHost16(saddr->sin_port));
        return true;
    } else if (addr.ss_family == AF_INET6) {
        const sockaddr_in6 *saddr =
            reinterpret_cast<const sockaddr_in6 *>(&addr);
        *out = SocketAddress(IPAddress(saddr->sin6_addr),
                             NetworkToHost16(saddr->sin6_port));
        return true;
    }

    return false;
}

}// namespace sled
