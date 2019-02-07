//
// Created by xi on 19-1-29.
//

#ifndef BLAZE_ENDIAN_H
#define BLAZE_ENDIAN_H

#include <stdint.h>
#include <endian.h>

namespace blaze
{

namespace net
{

namespace sockets
{

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"

inline uint64_t HostToNetwork64(uint64_t x)
{
    return htobe64(x);
}

inline uint32_t HostToNetwork32(uint32_t x)
{
    return htobe32(x);
}

inline uint16_t HostToNetwork16(uint16_t x)
{
    return htobe16(x);
}

inline uint64_t NetworkToHost64(uint64_t x)
{
    return be64toh(x);
}

inline uint32_t NetworkToHost32(uint64_t x)
{
    return be32toh(x);
}

inline uint16_t NetworkToHost16(uint16_t x)
{
    return be16toh(x);
}

#pragma GCC diagnostic pop

} // namespace sockets

} // namespace net

} // namespace blaze

#endif //BLAZE_ENDIAN_H
