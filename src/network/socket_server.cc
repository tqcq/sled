/**
 * @author      : {{NAME}} ({{EMAIL}})
 * @file        : {{FILE}}
 * @created     : {{TIMESTAMP}}
 */

#include "sled/network/socket_server.h"
#include "sled/network/physical_socket_server.h"

namespace sled {
constexpr TimeDelta SocketServer::kForever;

std::unique_ptr<sled::SocketServer>
CreateDefaultSocketServer()
{
    return std::unique_ptr<sled::SocketServer>(new PhysicalSocketServer());
}
}// namespace sled
