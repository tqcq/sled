/**
 * @file     : socket
 * @created  : Monday Feb 12, 2024 11:37:32 CST
 * @license  : MIT
 **/

#ifndef SLED_NETWORK_SOCKET_H
#define SLED_NETWORK_SOCKET_H
#pragma once

#include "sled/network/socket_address.h"
#include "sled/sigslot.h"
#include <cerrno>

#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

namespace sled {

inline bool
IsBlockingError(int e)
{
    return (e == EWOULDBLOCK || e == EAGAIN || e == EINPROGRESS);
}

class Socket {
public:
    virtual ~Socket() = default;

    Socket(const Socket &) = delete;
    Socket &operator=(const Socket &) = delete;

    virtual SocketAddress GetLocalAddress() const = 0;
    virtual SocketAddress GetRemoteAddress() const = 0;

    virtual int Bind(const SocketAddress &addr) = 0;
    virtual int Connect(const SocketAddress &addr) = 0;
    virtual int Send(const void *pv, size_t cb) = 0;
    virtual int SendTo(const void *pv, size_t cb, const SocketAddress &addr) = 0;
    virtual int Recv(void *pv, size_t cb, int64_t *timestamp) = 0;
    virtual int RecvFrom(void *pv, size_t cb, SocketAddress *paddr, int64_t *timestamp) = 0;
    virtual int Listen(int backlog) = 0;
    virtual Socket *Accept(SocketAddress *paddr) = 0;
    virtual int Close() = 0;
    virtual int GetError() const = 0;
    virtual void SetError(int error) = 0;

    inline bool IsBlocking() const { return IsBlockingError(GetError()); }

    enum ConnState { CS_CLOSED, CS_CONNECTING, CS_CONNECTED };

    virtual ConnState GetState() const = 0;

    enum Option {
        OPT_DONTFRAGMENT,
        OPT_RCVBUF,     // receive buffer size
        OPT_SNDBUF,     // send buffer size
        OPT_NODELAY,    // whether Nagle algorithm is enabled
        OPT_IPV6_V6ONLY,// Whether the socket is IPv6 only
        OPT_DSCP,       // DSCP code
        OPT_RTP_SENDTIME_EXTN_ID,
    };

    virtual int GetOption(Option opt, int *value) = 0;
    virtual int SetOption(Option opt, int value) = 0;
    sigslot::signal1<Socket *, sigslot::multi_threaded_local> SignalReadEvent;
    sigslot::signal1<Socket *, sigslot::multi_threaded_local> SignalWriteEvent;
    sigslot::signal1<Socket *> SignalConnectEvent;
    sigslot::signal2<Socket *, int> SignalCloseEvent;

protected:
    Socket() = default;
};

}// namespace sled

#endif// SLED_NETWORK_SOCKET_H
