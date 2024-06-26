#pragma once
#ifndef SLED_BYTE_ORDER_H
#define SLED_BYTE_ORDER_H
#include <arpa/inet.h>

#if defined(__APPLE__)

#include <libkern/OSByteOrder.h>

#define htobe16(x) OSSwapHostToBigInt16(x)
#define htole16(x) OSSwapHostToLittleInt16(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)

#define htobe32(x) OSSwapHostToBigInt32(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)

#define htobe64(x) OSSwapHostToBigInt64(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define be64toh(x) OSSwapBigToHostInt64(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)

#define __BYTE_ORDER BYTE_ORDER
#define __BIG_ENDIAN BIG_ENDIAN
#define __LITTLE_ENDIAN LITTLE_ENDIAN
#define __PDP_ENDIAN PDP_ENDIAN
#elif defined(__EMSCRIPTEN__)
#define htobe16(x) __builtin_bswap16(x)
#define htole16(x) (x)
#define be16toh(x) __builtin_bswap16(x)
#define le16toh(x) (x)

#define htobe32(x) __builtin_bswap32(x)
#define htole32(x) (x)
#define be32toh(x) __builtin_bswap32(x)
#define le32toh(x) (x)

#define htobe64(x) __builtin_bswap64(x)
#define htole64(x) (x)
#define be64toh(x) __builtin_bswap64(x)
#define le64toh(x) (x)
// #define __BYTE_ORDER __BYTE_ORDER__
// #define __BIG_ENDIAN __ORDER_BIG_ENDIAN__
// #define __LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
// #define __PDP_ENDIAN __ORDER_PDP_ENDIAN__
#endif

inline uint16_t
HostToNetwork16(uint16_t n)
{
    return htobe16(n);
}

inline uint32_t
HostToNetwork32(uint32_t n)
{
    return htobe32(n);
}

inline uint16_t
NetworkToHost16(uint16_t n)
{
    return be16toh(n);
}

inline uint32_t
NetworkToHost32(uint32_t n)
{
    return be32toh(n);
}

#endif// SLED_BYTE_ORDER_H
