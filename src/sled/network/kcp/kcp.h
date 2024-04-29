#ifndef SLED_NETWORK_KCP_KCP_H
#define SLED_NETWORK_KCP_KCP_H

#pragma once

#include <string>
#include <unistd.h>

/**
 kcp header
0               4   5   6       8 (BYTE)
+---------------+---+---+-------+
|     conv      |cmd|frg|  wnd  |
+---------------+---+---+-------+   8
|     ts        |     sn        |
+---------------+---------------+  16
|     una       |     len       |
+---------------+---------------+  24
|                               |
|        DATA (optional)        |
|                               |
+-------------------------------+
*/
namespace sled {

enum class KCPCommand {
    kPush,
    kAck,
    kWask,
    kWins,
};

struct KCPHeader {
    uint32_t conv;// conv id
    uint8_t cmd;  // Command
    uint8_t frg;  // fragment number
    uint16_t wnd; // my available window size

    uint32_t ts; // timestamp
    uint32_t sn; // sequence number
    uint32_t una;// sender unacknowledged (min) number
    uint32_t len;// data len
};

struct KCPPacket {
    KCPHeader header;
    char data[1];
};

}// namespace sled

#endif// SLED_NETWORK_KCP_KCP_H
