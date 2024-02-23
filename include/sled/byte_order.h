#include <arpa/inet.h>

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
