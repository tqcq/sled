/**
 * @file     : physical_socket_server
 * @created  : Monday Feb 19, 2024 11:35:13 CST
 * @license  : MIT
 **/

#ifndef SLED_NETWORK_PHYSICAL_SOCKET_SERVER_H
#define SLED_NETWORK_PHYSICAL_SOCKET_SERVER_H
#pragma once

#include "sled/network/async_resolver.h"
#include "sled/sigslot.h"
#include "sled/synchronization/mutex.h"
#include "socket.h"
#include "socket_server.h"
#include <unordered_map>

typedef int SOCKET;

namespace sled {

enum DispatcherEvent {
    DE_READ = 0x0001,
    DE_WRITE = 0x0002,
    DE_CONNECT = 0x0004,
    DE_CLOSE = 0x0008,
    DE_ACCEPT = 0x0010,
};

class Signaler;

class Dispatcher {
public:
    virtual ~Dispatcher() = default;
    virtual uint32_t GetRequestedEvents() = 0;
    virtual void OnEvent(uint32_t ff, int err) = 0;

    virtual int GetDescriptor() = 0;
    virtual bool IsDescriptorClosed() = 0;
};

class PhysicalSocketServer : public SocketServer {
public:
    PhysicalSocketServer();
    ~PhysicalSocketServer() override;
    Socket *CreateSocket(int family, int type) override;
    virtual Socket *WrapSocket(SOCKET s);
    bool Wait(TimeDelta max_wait_duration, bool process_io) override;
    void WakeUp() override;

    void Add(Dispatcher *dispatcher);
    void Remove(Dispatcher *dispatcher);
    void Update(Dispatcher *dispatcher);

private:
    static const int kForeverMs = -1;
    static int ToCusWait(TimeDelta max_wait_duration);

    bool WaitSelect(int64_t cusWait, bool process_io);

    uint64_t next_dispatcher_key_ = 0;
    std::unordered_map<uint64_t, Dispatcher *> dispatcher_by_key_ GUARDED_BY(lock_);
    std::unordered_map<Dispatcher *, uint64_t> key_by_dispatcher_ GUARDED_BY(lock_);
    std::vector<uint64_t> current_dispatcher_keys_ GUARDED_BY(lock_);
    Signaler *signal_wakeup_;
    // Mutex lock_;
    RecursiveMutex lock_;

    bool fWait_;
    bool waiting_ = false;
};

class PhysicalSocket : public Socket, public sigslot::has_slots<> {
public:
    PhysicalSocket(PhysicalSocketServer *ss, SOCKET s = INVALID_SOCKET);
    ~PhysicalSocket() override;
    virtual bool Create(int family, int type);
    SocketAddress GetLocalAddress() const override;
    SocketAddress GetRemoteAddress() const override;
    int Bind(const SocketAddress &bind_addr) override;
    int Connect(const SocketAddress &addr) override;

    int GetError() const override;
    void SetError(int error) override;

    ConnState GetState() const override { return state_; };

    int GetOption(Option opt, int *value) override;
    int SetOption(Option opt, int value) override;

    int Send(const void *pv, size_t cb) override;
    int SendTo(const void *pv, size_t cb, const SocketAddress &addr) override;
    int Recv(void *pv, size_t cb, int64_t *timestamp) override;
    int RecvFrom(void *pv, size_t cb, SocketAddress *paddr, int64_t *timestamp) override;
    int Listen(int backlog) override;
    Socket *Accept(SocketAddress *paddr) override;

    int Close() override;

    SocketServer *socketserver() { return ss_; }

    SOCKET GetSocketFD() const { return s_; }

protected:
    int DoConnect(const SocketAddress &addr);
    virtual SOCKET DoAccept(SOCKET socket, sockaddr *addr, socklen_t *addrlen);
    virtual int DoSend(SOCKET socket, const char *buf, int len, int flags);
    virtual int
    DoSendTo(SOCKET socket, const char *buf, int len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
    int DoReadFromSocket(void *buffer, size_t length, SocketAddress *out_addr, int64_t *timestamp);

    void OnResolveResult(AsyncResolverInterface *resolver);
    void UpdateLastError();

    uint8_t enabled_events() const { return enabled_events_; }

    virtual void SetEnabledEvents(uint8_t events);
    virtual void EnableEvents(uint8_t events);
    virtual void DisableEvents(uint8_t events);
    int TranslateOption(Option opt, int *slevel, int *sopt);

    PhysicalSocketServer *ss_;
    SOCKET s_;
    bool udp_;
    int family_ = 0;
    mutable Mutex mutex_;
    int error_ GUARDED_BY(mutex_);
    ConnState state_;
    AsyncResolver *resolver_;

private:
    uint8_t enabled_events_ = 0;
};

class SocketDispatcher : public Dispatcher, public PhysicalSocket {
public:
    explicit SocketDispatcher(PhysicalSocketServer *ss);
    SocketDispatcher(SOCKET s, PhysicalSocketServer *ss);
    ~SocketDispatcher() override;

    bool Initialize();

    virtual bool Create(int type);
    bool Create(int family, int type) override;

    int GetDescriptor() override;
    bool IsDescriptorClosed() override;

    uint32_t GetRequestedEvents() override;
    void OnEvent(uint32_t ff, int err) override;

    int Close() override;
};

}// namespace sled

#endif// SLED_NETWORK_PHYSICAL_SOCKET_SERVER_H
