#include "sled/network/async_resolver.h"
#include "sled/network/async_resolver_interface.h"
#include "sled/ref_counted_base.h"
#include "sled/synchronization/mutex.h"
#include "sled/task_queue/task_queue_base.h"
#include <thread>

namespace sled {
struct AsyncResolver::State : public RefCountedBase {
    Mutex mutex;
    enum class Status {
        kLive,
        kDead,
    } status = Status::kLive;
};

int
ResolveHostname(const std::string &hostname,
                int family,
                std::vector<IPAddress> *addresses)
{
    if (!addresses) { return -1; }

    addresses->clear();
    struct addrinfo *result = nullptr;
    struct addrinfo hints = {0};
    hints.ai_family = family;
    hints.ai_flags = AI_ADDRCONFIG;
    int ret = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (ret != 0) { return ret; }

    struct addrinfo *cursor = result;
    for (; cursor; cursor = cursor->ai_next) {
        if (family == AF_UNSPEC || cursor->ai_family == family) {
            IPAddress ip;
            if (IPFromAddrInfo(cursor, &ip)) { addresses->push_back(ip); }
        }
    }
    freeaddrinfo(result);
    return 0;
}

AsyncResolver::AsyncResolver() : error_(-1), state_(new State) {}

AsyncResolver::~AsyncResolver()
{
    MutexLock lock(&state_->mutex);
    state_->status = State::Status::kDead;
}

void
AsyncResolver::Start(const SocketAddress &addr)
{
    Start(addr, addr.family());
}

void
AsyncResolver::Start(const SocketAddress &addr, int family)
{
    addr_ = addr;

    auto caller_task_queue = TaskQueueBase::Current();
    auto state = state_;
    auto thread_function = [this, addr, family, caller_task_queue, state] {
        std::vector<IPAddress> addresses;
        int error = ResolveHostname(addr.hostname(), family, &addresses);
        caller_task_queue->PostTask([this, error, &addresses, state] {
            bool live;
            {
                MutexLock lock(&state->mutex);
                live = state->status == State::Status::kLive;
            }
            if (live) { ResolveDone(std::move(addresses), error); }
        });
    };
    // TODO: Add new thread run function
    std::thread(thread_function).detach();
}

bool
AsyncResolver::GetResolvedAddress(int family, SocketAddress *addr) const
{
    if (error_ != 0 || addresses_.empty()) { return false; }

    *addr = addr_;
    for (size_t i = 0; i < addresses_.size(); ++i) {
        if (family == addresses_[i].family()) {
            addr->SetResolvedIP(addresses_[i]);
            return true;
        }
    }
    return false;
}

int
AsyncResolver::GetError() const
{
    return error_;
}

void
AsyncResolver::Destroy(bool wait)
{
    destroy_called_ = true;
    MaybeSelfDestruct();
}

const std::vector<IPAddress> &
AsyncResolver::addresses() const
{
    return addresses_;
}

void
AsyncResolver::ResolveDone(std::vector<IPAddress> addresses, int error)
{
    addresses_ = addresses;
    error_ = error;
    recursion_check_ = true;
    SignalDone(this);
    MaybeSelfDestruct();
}

void
AsyncResolver::MaybeSelfDestruct()
{
    if (!recursion_check_) {
        delete this;
    } else {
        recursion_check_ = false;
    }
}
}// namespace sled
