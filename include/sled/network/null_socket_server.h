/**
 * @file     : null_socket_server
 * @created  : Wednesday Feb 14, 2024 13:36:48 CST
 * @license  : MIT
 **/

#ifndef NULL_SOCKET_SERVER_H
#define NULL_SOCKET_SERVER_H

#include "sled/network/socket_server.h"
#include "sled/synchronization/event.h"

namespace sled {

class NullSocketServer : public SocketServer {
public:
    NullSocketServer();
    ~NullSocketServer() override;
    bool Wait(TimeDelta max_wait_duration, bool process_io) override;
    void WakeUp() override;

    Socket *CreateSocket(int family, int type) override;
private:
    Event event_;
};

}// namespace sled

#endif// NULL_SOCKET_SERVER_H
