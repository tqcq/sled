/**
 * @file     : socket_server
 * @created  : Monday Feb 12, 2024 11:36:32 CST
 * @license  : MIT
 **/

#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include "sled/network/socket_factory.h"
#include "sled/units/time_delta.h"
#include <memory>

namespace sled {

class Thread;

class SocketServer : public SocketFactory {
public:
    static constexpr TimeDelta kForever = TimeDelta::PlusInfinity();

    virtual void SetMessageQueue(Thread *queue) {}

    virtual bool Wait(TimeDelta max_wait_duration, bool process_io) = 0;

    virtual void WakeUp() = 0;
};

std::unique_ptr<sled::SocketServer> CreateDefaultSocketServer();

}// namespace sled

#endif// SOCKET_SERVER_H
