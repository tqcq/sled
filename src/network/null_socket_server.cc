
#include "sled/network/null_socket_server.h"

namespace sled {
NullSocketServer::NullSocketServer() = default;

NullSocketServer::~NullSocketServer() {}

bool
NullSocketServer::Wait(TimeDelta max_wait_duration, bool process_io)
{
    event_.Wait(max_wait_duration);
    return true;
}

void
NullSocketServer::WakeUp()
{
    event_.Set();
}

Socket *
NullSocketServer::CreateSocket(int family, int type)
{
    return nullptr;
}
}// namespace sled
