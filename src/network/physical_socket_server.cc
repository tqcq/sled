#include "sled/network/physical_socket_server.h"
#include "sled/log/log.h"
#include "sled/network/async_resolver.h"
#include "sled/network/socket.h"
#include "sled/synchronization/event.h"
#include "sled/synchronization/mutex.h"
#include "sled/time_utils.h"
#include <array>
#include <errno.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
class ScopedSetTrue {
public:
    ScopedSetTrue(bool *value) : value_(value) { *value_ = true; }

    ~ScopedSetTrue() { *value_ = false; }

private:
    bool *value_;
};
}// namespace

namespace sled {
class Signaler : public Dispatcher {
public:
    Signaler(PhysicalSocketServer *ss, bool &flag_to_clear)
        : ss_(ss),
          afd_([] {
              std::array<int, 2> afd = {INVALID_SOCKET, INVALID_SOCKET};
              if (pipe(afd.data()) < 0) {
                  // TODO: add log here
              }
              return afd;
          }()),
          fSignaled_(false),
          flag_to_clear_(flag_to_clear)
    {
        ss_->Add(this);
    }

    ~Signaler() override
    {
        ss_->Remove(this);
        close(afd_[0]);
        close(afd_[1]);
    }

    virtual void Signal()
    {
        MutexLock lock(&mutex_);
        if (!fSignaled_) {
            const uint8_t b[1] = {0};
            const ssize_t res = write(afd_[1], b, sizeof(b));
            fSignaled_ = true;
        }
    }

    uint32_t GetRequestedEvents() override { return DE_READ; }

    void OnEvent(uint32_t ff, int err) override
    {
        MutexLock lock(&mutex_);
        if (fSignaled_) {
            uint8_t b[4];
            const ssize_t res = read(afd_[0], b, sizeof(b));
            fSignaled_ = false;
        }
        flag_to_clear_ = false;
    }

    int GetDescriptor() override { return afd_[0]; }

    bool IsDescriptorClosed() override { return false; }

private:
    PhysicalSocketServer *const ss_;
    const std::array<int, 2> afd_;
    bool fSignaled_;
    Mutex mutex_;
    bool &flag_to_clear_;
};

PhysicalSocketServer::PhysicalSocketServer() : fWait_(false)
{
    signal_wakeup_ = new Signaler(this, fWait_);
}

PhysicalSocketServer::~PhysicalSocketServer() { delete signal_wakeup_; }

void
PhysicalSocketServer::WakeUp()
{
    signal_wakeup_->Signal();
}

Socket *
PhysicalSocketServer::CreateSocket(int family, int type)
{
    SocketDispatcher *dispatcher = new SocketDispatcher(this);
    if (dispatcher->Create(family, type)) {
        return dispatcher;
    } else {
        delete dispatcher;
        return nullptr;
    }
}

Socket *
PhysicalSocketServer::WrapSocket(SOCKET s)
{
    SocketDispatcher *dispatcher = new SocketDispatcher(s, this);
    if (dispatcher->Initialize()) {
        return dispatcher;
    } else {
        delete dispatcher;
        return nullptr;
    }
}

void
PhysicalSocketServer::Add(Dispatcher *pdispatcher)
{
    RecursiveMutexLock lock(&lock_);
    if (key_by_dispatcher_.count(pdispatcher)) { return; }

    uint64_t key = next_dispatcher_key_++;
    dispatcher_by_key_.emplace(key, pdispatcher);
    key_by_dispatcher_.emplace(pdispatcher, key);
}

void
PhysicalSocketServer::Remove(Dispatcher *pdispatcher)
{
    RecursiveMutexLock lock(&lock_);
    if (!key_by_dispatcher_.count(pdispatcher)) { return; }

    uint64_t key = key_by_dispatcher_.at(pdispatcher);
    key_by_dispatcher_.erase(pdispatcher);
    dispatcher_by_key_.erase(key);
}

void
PhysicalSocketServer::Update(Dispatcher *pdispatcher)
{
    return;
    /*
    MutexLock lock(&lock_);
    if (!key_by_dispatcher_.count(pdispatcher)) { return; }
    UpdateEpoll();
    */
}

int
PhysicalSocketServer::ToCusWait(TimeDelta max_wait_duration)
{
    return max_wait_duration == Event::kForever
        ? kForeverMs
        : max_wait_duration.RoundUpTo(TimeDelta::Micros(1)).us();
}

bool
PhysicalSocketServer::Wait(TimeDelta max_wait_duration, bool process_io)

{
    ScopedSetTrue s(&waiting_);
    const int64_t cusWait = ToCusWait(max_wait_duration);

    return WaitSelect(cusWait, process_io);
}

static void
ProcessEvents(Dispatcher *pdispatcher,
              bool readable,
              bool writable,
              bool error_event,
              bool check_error)
{
    int errcode = 0;
    if (check_error) {
        socklen_t len = sizeof(errcode);
        int res = ::getsockopt(pdispatcher->GetDescriptor(), SOL_SOCKET,
                               SO_ERROR, &errcode, &len);
        if (res < 0) {
            if (error_event || errno != ENOTSOCK) { errcode = EBADF; }
        }
    }

    const uint32_t requested_events = pdispatcher->GetRequestedEvents();
    uint32_t ff = 0;

    if (readable) {
        if (errcode || pdispatcher->IsDescriptorClosed()) {
            ff |= DE_CLOSE;
        } else if (requested_events & DE_ACCEPT) {
            ff |= DE_ACCEPT;
        } else {
            ff |= DE_READ;
        }
    }

    if (writable) {
        if (requested_events & DE_CONNECT) {
            if (!errcode) { ff |= DE_CONNECT; }
        } else {
            ff |= DE_WRITE;
        }
    }

    if (errcode) { ff |= DE_CLOSE; }

    if (ff != 0) { pdispatcher->OnEvent(ff, errcode); }
}

bool
PhysicalSocketServer::WaitSelect(int64_t cusWait, bool process_io)
{
    struct timeval *ptvWait = nullptr;
    struct timeval tvWait;
    int64_t stop_us;
    if (cusWait != kForeverMs) {
        tvWait.tv_sec = cusWait / sled::kNumMicrosecsPerSec;
        tvWait.tv_usec = (cusWait % sled::kNumMicrosecsPerSec);
        ptvWait = &tvWait;
        stop_us = TimeMicros() + cusWait;
    }

    fd_set fdsRead;
    fd_set fdsWrite;

    fWait_ = true;
    while (fWait_) {
        FD_ZERO(&fdsRead);
        FD_ZERO(&fdsWrite);
        int fdmax = -1;
        {
            RecursiveMutexLock lock(&lock_);
            current_dispatcher_keys_.clear();
            for (auto const &kv : dispatcher_by_key_) {
                uint64_t key = kv.first;
                Dispatcher *pdispatcher = kv.second;
                if (!process_io && (pdispatcher != signal_wakeup_)) {
                    continue;
                }
                current_dispatcher_keys_.push_back(key);
                int fd = pdispatcher->GetDescriptor();
                if (fd > fdmax) { fdmax = fd; }

                uint32_t ff = pdispatcher->GetRequestedEvents();
                if (ff & (DE_READ | DE_ACCEPT)) { FD_SET(fd, &fdsRead); }

                if (ff & (DE_WRITE | DE_CONNECT)) { FD_SET(fd, &fdsWrite); }
            }
        }

        int n = select(fdmax + 1, &fdsRead, &fdsWrite, nullptr, ptvWait);

        if (n < 0) {
            if (errno != EINTR) { return false; }
        } else if (n == 0) {
            return true;
        } else {
            RecursiveMutexLock lock(&lock_);
            for (uint64_t key : current_dispatcher_keys_) {
                // skip if the dispatcher is removed
                if (!dispatcher_by_key_.count(key)) { continue; }

                Dispatcher *pdispatcher = dispatcher_by_key_.at(key);
                int fd = pdispatcher->GetDescriptor();

                bool readable = FD_ISSET(fd, &fdsRead);
                if (readable) { FD_CLR(fd, &fdsRead); }

                bool writable = FD_ISSET(fd, &fdsWrite);
                if (writable) { FD_CLR(fd, &fdsWrite); }

                ProcessEvents(pdispatcher, readable, writable, false,
                              readable || writable);
            }
        }

        if (ptvWait) {
            ptvWait->tv_sec = 0;
            ptvWait->tv_usec = 0;
            int64_t time_left_us = stop_us - TimeMicros();
            if (time_left_us > 0) {
                ptvWait->tv_sec = time_left_us / kNumMicrosecsPerSec;
                ptvWait->tv_usec = time_left_us % kNumMicrosecsPerSec;
            } else {
                break;
            }
        }
    }
    return true;
}

PhysicalSocket::PhysicalSocket(PhysicalSocketServer *ss, SOCKET s)
    : ss_(ss),
      s_(s),
      error_(0),
      state_((s == INVALID_SOCKET) ? CS_CLOSED : CS_CONNECTED),
      resolver_(nullptr)
{
    if (s != INVALID_SOCKET) {
        SetEnabledEvents(DE_READ | DE_WRITE);
        int type = SOCK_STREAM;
        socklen_t len = sizeof(type);
        const int res =
            getsockopt(s_, SOL_SOCKET, SO_TYPE, (void *) &type, &len);
        udp_ = (SOCK_DGRAM == type);
    }
}

PhysicalSocket::~PhysicalSocket() { Close(); }

bool
PhysicalSocket::Create(int family, int type)
{
    Close();

    s_ = ::socket(family, type, 0);
    udp_ = (SOCK_DGRAM == type);
    family_ = family;
    UpdateLastError();
    if (udp_) { SetEnabledEvents(DE_READ | DE_WRITE); }
    return s_ != INVALID_SOCKET;
}

SocketAddress
PhysicalSocket::GetLocalAddress() const
{
    sockaddr_storage addr_storage = {};
    socklen_t addrlen = sizeof(addr_storage);
    sockaddr *addr = reinterpret_cast<sockaddr *>(&addr_storage);
    int result = ::getsockname(s_, addr, &addrlen);
    SocketAddress address;
    if (result >= 0) {
    } else {
    }
    return address;
}

SocketAddress
PhysicalSocket::GetRemoteAddress() const
{
    sockaddr_storage addr_storage = {};
    socklen_t addrlen = sizeof(addr_storage);
    sockaddr *addr = reinterpret_cast<sockaddr *>(&addr_storage);
    int result = ::getpeername(s_, addr, &addrlen);
    SocketAddress address;
    if (result >= 0) {
    } else {
    }
    return address;
}

int
PhysicalSocket::Bind(const SocketAddress &bind_addr)
{
    SocketAddress copied_bind_addr = bind_addr;
    sockaddr_storage addr_storage;
    size_t len = copied_bind_addr.ToSockAddrStorage(&addr_storage);
    sockaddr *addr = reinterpret_cast<sockaddr *>(&addr_storage);
    int err = ::bind(s_, addr, static_cast<int>(len));
    UpdateLastError();
    return err;
}

int
PhysicalSocket::Connect(const SocketAddress &addr)
{
    if (state_ != CS_CLOSED) {
        SetError(EALREADY);
        return SOCKET_ERROR;
    }
    if (addr.IsUnresolvedIP()) {
        resolver_ = new AsyncResolver();
        resolver_->SignalDone.connect(this, &PhysicalSocket::OnResolveResult);
        resolver_->Start(addr);
        state_ = CS_CONNECTING;
        return 0;
    }

    return DoConnect(addr);
}

int
PhysicalSocket::GetError() const
{
    MutexLock lock(&mutex_);
    return error_;
}

void

PhysicalSocket::SetError(int error)
{
    // MutexLock lock(&mutex_);
    LockGuard<Mutex> lock(&mutex_);
    error_ = error;
}

int
PhysicalSocket::GetOption(Option opt, int *value)
{
    int slevel;
    int sopt;
    if (TranslateOption(opt, &slevel, &sopt) == -1) { return -1; }
    socklen_t optlen = sizeof(*value);
    int ret = ::getsockopt(s_, slevel, sopt, (void *) value, &optlen);
    if (ret == -1) { return -1; }

    return ret;
}

int
PhysicalSocket::SetOption(Option opt, int value)
{
    int slevel;
    int sopt;
    if (TranslateOption(opt, &slevel, &sopt) == -1) { return -1; }
    int result = ::setsockopt(s_, slevel, sopt, (void *) &value, sizeof(value));
    if (result != 0) { UpdateLastError(); }
    return result;
}

int
PhysicalSocket::Send(const void *pv, size_t cb)
{
    int sent = DoSend(s_, reinterpret_cast<const char *>(pv),
                      static_cast<int>(cb), MSG_NOSIGNAL);
    UpdateLastError();
    if ((sent > 0 && sent < static_cast<int>(cb))
        || (sent < 0 && IsBlockingError(GetError()))) {
        EnableEvents(DE_WRITE);
    }
    return sent;
}

int
PhysicalSocket::SendTo(const void *buffer,
                       size_t length,
                       const SocketAddress &addr)
{
    sockaddr_storage saddr;
    size_t len = addr.ToSockAddrStorage(&saddr);

    int sent =
        DoSendTo(s_, static_cast<const char *>(buffer),
                 static_cast<int>(length), MSG_NOSIGNAL,
                 reinterpret_cast<sockaddr *>(&saddr), static_cast<int>(len));
    UpdateLastError();
    if ((sent > 0 && sent < static_cast<int>(length))
        || (sent < 0 && IsBlockingError(GetError()))) {
        EnableEvents(DE_WRITE);
    }
    return sent;
}

int
PhysicalSocket::Recv(void *buffer, size_t length, int64_t *timestamp)
{
    int received = DoReadFromSocket(buffer, length, nullptr, timestamp);
    if ((received == 0) && (length != 0)) {
        EnableEvents(DE_READ);
        SetError(EWOULDBLOCK);
        return SOCKET_ERROR;
    }

    UpdateLastError();
    int error = GetError();
    bool success = (received >= 0) || IsBlockingError(GetError());
    if (udp_ || success) { EnableEvents(DE_READ); }
    if (!success) {
        // TODO: add log here
    }

    return received;
}

int
PhysicalSocket::RecvFrom(void *buffer,
                         size_t length,
                         SocketAddress *out_addr,
                         int64_t *timestamp)
{
    int received = DoReadFromSocket(buffer, length, out_addr, timestamp);
    UpdateLastError();
    int error = GetError();
    bool success = (received >= 0) || IsBlockingError(error);
    if (udp_ || success) { EnableEvents(DE_READ); }

    if (!success) {
        // TODO: add log here
    }

    return received;
}

int64_t
GetSocketRecvTimestamp(int socket)
{
    struct timeval tv_ioctl;

#if defined(SIOCGSTAMP_OLD)
    int ret = ioctl(socket, SIOCGSTAMP_OLD, &tv_ioctl);
#elif defined(SIOCGSTAMP)
    int ret = ioctl(socket, SIOCGSTAMP, &tv_ioctl);
#else
    int ret = -1;
#endif

    if (ret != 0) { return -1; }

    int64_t timestamp =
        static_cast<int64_t>(tv_ioctl.tv_sec) * kNumMicrosecsPerSec
        + tv_ioctl.tv_usec;
    return timestamp;
}

int
PhysicalSocket::DoReadFromSocket(void *buffer,
                                 size_t length,
                                 SocketAddress *out_addr,
                                 int64_t *timestamp)
{
    sockaddr_storage addr_storage;
    socklen_t addr_len = sizeof(addr_storage);
    sockaddr *addr = reinterpret_cast<sockaddr *>(&addr_storage);

    int received = 0;
    if (out_addr) {
        received = ::recvfrom(s_, static_cast<char *>(buffer),
                              static_cast<int>(length), 0, addr, &addr_len);
        SocketAddressFromSockAddrStorage(addr_storage, out_addr);
    } else {
        received = ::recv(s_, static_cast<char *>(buffer),
                          static_cast<int>(length), 0);
    }
    if (timestamp) { *timestamp = GetSocketRecvTimestamp(s_); }

    return received;
}

int
PhysicalSocket::Listen(int backlog)
{
    int err = ::listen(s_, backlog);
    UpdateLastError();

    if (err == 0) {
        state_ = CS_CONNECTING;
        EnableEvents(DE_ACCEPT);
    }
    return err;
}

Socket *
PhysicalSocket::Accept(SocketAddress *out_addr)
{
    EnableEvents(DE_ACCEPT);
    sockaddr_storage addr_storage;
    socklen_t addr_len = sizeof(addr_storage);
    sockaddr *addr = reinterpret_cast<sockaddr *>(&addr_storage);
    SOCKET s = DoAccept(s_, addr, &addr_len);
    UpdateLastError();
    if (s == INVALID_SOCKET) { return nullptr; }
    if (out_addr) { SocketAddressFromSockAddrStorage(addr_storage, out_addr); }

    return ss_->WrapSocket(s);
}

int
PhysicalSocket::Close()
{
    if (s_ == INVALID_SOCKET) { return 0; }

    int err = ::close(s_);
    UpdateLastError();
    s_ = INVALID_SOCKET;
    state_ = CS_CLOSED;
    SetEnabledEvents(0);
    if (resolver_) {
        resolver_->Destroy(false);
        resolver_ = nullptr;
    }
    return err;
}

SOCKET
PhysicalSocket::DoAccept(SOCKET socket, sockaddr *addr, socklen_t *addrlen)
{
    return ::accept(socket, addr, addrlen);
}

int
PhysicalSocket::DoSend(SOCKET socket, const char *buf, int len, int flags)
{
    return ::send(socket, buf, len, flags);
}

int
PhysicalSocket::DoSendTo(SOCKET socket,
                         const char *buf,
                         int len,
                         int flags,
                         const struct sockaddr *dest_addr,
                         socklen_t addrlen)
{
    return ::sendto(socket, buf, len, flags, dest_addr, addrlen);
}

int
PhysicalSocket::DoConnect(const SocketAddress &connect_addr)
{
    if ((s_ == INVALID_SOCKET) && !Create(connect_addr.family(), SOCK_STREAM)) {
        return SOCKET_ERROR;
    }

    sockaddr_storage addr_storage;
    size_t len = connect_addr.ToSockAddrStorage(&addr_storage);
    sockaddr *addr = reinterpret_cast<sockaddr *>(&addr_storage);
    int err = ::connect(s_, addr, static_cast<int>(len));
    UpdateLastError();
    uint8_t events = DE_READ | DE_WRITE;
    if (err == 0) {
        state_ = CS_CONNECTED;
    } else if (IsBlockingError(GetError())) {
        state_ = CS_CONNECTING;
        events |= DE_CONNECT;
    } else {
        return SOCKET_ERROR;
    }

    EnableEvents(events);
    return 0;
}

void
PhysicalSocket::OnResolveResult(AsyncResolverInterface *resolver)
{
    if (resolver != resolver_) { return; }

    int error = resolver_->GetError();
    if (error == 0) {
        error = DoConnect(resolver_->address());
    } else {
        // get error from resolver
        Close();
        SetError(error);
        SignalCloseEvent(this, error);
    }
}

void
PhysicalSocket::UpdateLastError()
{
    SetError(errno);
}

void
PhysicalSocket::SetEnabledEvents(uint8_t events)
{
    enabled_events_ = events;
}

void
PhysicalSocket::EnableEvents(uint8_t events)
{
    enabled_events_ |= events;
}

void
PhysicalSocket::DisableEvents(uint8_t events)
{
    enabled_events_ &= ~events;
}

int
PhysicalSocket::TranslateOption(Option opt, int *slevel, int *sopt)
{
    switch (opt) {
    case OPT_RCVBUF:
        *slevel = SOL_SOCKET;
        *sopt = SO_RCVBUF;
        break;
    case OPT_SNDBUF:
        *slevel = SOL_SOCKET;
        *sopt = SO_SNDBUF;
        break;
    case OPT_NODELAY:
        *slevel = IPPROTO_TCP;
        *sopt = TCP_NODELAY;
        break;
    default:
        return -1;
    }
    return 0;
}

SocketDispatcher::SocketDispatcher(PhysicalSocketServer *ss)
    : PhysicalSocket(ss)
{}

SocketDispatcher::SocketDispatcher(SOCKET s, PhysicalSocketServer *ss)
    : PhysicalSocket(ss, s)
{}

SocketDispatcher::~SocketDispatcher() { Close(); }

bool
SocketDispatcher::Initialize()
{
    // must be a non-blocking
    fcntl(s_, F_SETFL, fcntl(s_, F_GETFL, 0) | O_NONBLOCK);
    ss_->Add(this);
    return true;
}

bool
SocketDispatcher::Create(int type)
{
    return Create(AF_INET, type);
}

bool
SocketDispatcher::Create(int family, int type)
{
    if (!PhysicalSocket::Create(family, type)) { return false; }

    if (!Initialize()) { return false; }

    return true;
}

int
SocketDispatcher::GetDescriptor()
{
    return s_;
}

bool
SocketDispatcher::IsDescriptorClosed()
{
    if (udp_) { return s_ == INVALID_SOCKET; }

    char ch;
    ssize_t res;
    do {
        res = ::recv(s_, &ch, 1, MSG_PEEK);
    } while (res < 0 && errno == EINTR);

    if (res > 0) {
        return false;
    } else if (res == 0) {
        return true;
    } else {
        switch (errno) {
        case EBADF:
            return true;
        case ECONNRESET:
            return true;
        case ECONNABORTED:
            return true;
        case EPIPE:
            return true;
        case EWOULDBLOCK:
            return false;
        default:
            // Assuming benign blocking error
            return false;
        }
    }
}

uint32_t
SocketDispatcher::GetRequestedEvents()
{
    return enabled_events();
}

void
SocketDispatcher::OnEvent(uint32_t ff, int err)
{
    if ((ff & DE_CONNECT) != 0) { state_ = CS_CONNECTED; }

    if ((ff & DE_CLOSE) != 0) { state_ = CS_CLOSED; }

    if ((ff & DE_CONNECT) != 0) {
        DisableEvents(DE_CONNECT);
        SignalConnectEvent(this);
    }

    if ((ff & DE_ACCEPT) != 0) {
        DisableEvents(DE_ACCEPT);
        SignalReadEvent(this);
    }

    if ((ff & DE_READ) != 0) {
        DisableEvents(DE_READ);
        SignalReadEvent(this);
    }

    if ((ff & DE_WRITE) != 0) {
        DisableEvents(DE_WRITE);
        SignalWriteEvent(this);
    }

    if ((ff & DE_CLOSE) != 0) {
        SetEnabledEvents(0);
        SignalCloseEvent(this, err);
    }
}

int
SocketDispatcher::Close()
{
    if (s_ == INVALID_SOCKET) { return 0; }

    ss_->Remove(this);
    return PhysicalSocket::Close();
}

}// namespace sled
