//
// Created by xi on 19-1-31.
//

#include <netdb.h>

#include <blaze/log/Logging.h>
#include <blaze/net/Endian.h>
#include <blaze/net/SocketsOps.h>

#include <blaze/net/InetAddress.h>

// INADDR_ANY use old style (type) value casting

#pragma clang diagnostic ignored "-Wold-style-cast"
static const in_addr_t kInAddrAny = INADDR_ANY;
static const in_addr_t kInAddrLoopback = INADDR_LOOPBACK;
#pragma clang diagnostic error "-Wold-style-cast"

//     /* Structure describing an Internet socket address.  */
//     struct sockaddr_in {
//         sa_family_t    sin_family; /* address family: AF_INET */
//         uint16_t       sin_port;   /* port in network byte order */
//         struct in_addr sin_addr;   /* internet address */
//     };

//     /* Internet address. */
//     typedef uint32_t in_addr_t;
//     struct in_addr {
//         in_addr_t       s_addr;     /* address in network byte order */
//     };

//     struct sockaddr_in6 {
//         sa_family_t     sin6_family;   /* address family: AF_INET6 */
//         uint16_t        sin6_port;     /* port in network byte order */
//         uint32_t        sin6_flowinfo; /* IPv6 flow information */
//         struct in6_addr sin6_addr;     /* IPv6 address */
//         uint32_t        sin6_scope_id; /* IPv6 scope-id */
//     };

namespace blaze
{
namespace net
{

// #define offsetof(type, member) (size_t)&(((type*)0)->member)

// in linux/kernel.h
// #define container_of(ptr, type, member) ({ \
//      const typeof( ((type *)0)->member ) *__mptr = (ptr); \
//      (type *)( (char *)__mptr - offsetof(type,member) );})

// https://www.cnblogs.com/Anker/p/3472271.html

static_assert(sizeof(InetAddress) == sizeof(struct sockaddr_in6), "InetAddress is same size as sockaddr_in6");
static_assert(offsetof(sockaddr_in, sin_family) == 0, "sin_family offset is 0");
static_assert(offsetof(sockaddr_in6, sin6_family) == 0, "sin6_family offset is 0");
static_assert(offsetof(sockaddr_in, sin_port) == 2, "sin_port offset is 2");
static_assert(offsetof(sockaddr_in6, sin6_port) == 2, "sin6_port offset is 2");

InetAddress::InetAddress(uint16_t port, bool loopback_only, bool ipv6)
{
    static_assert(offsetof(InetAddress, addr_) == 0, "addr offset is 0");
    static_assert(offsetof(InetAddress, addr6_) == 0, "addr6 offset is 0");
    if (ipv6)
    {
        bzero(&addr6_, sizeof(addr6_));
        addr6_.sin6_family = AF_INET6;

        // structure of in6_addr
//        struct in6_addr
//        {
//            union
//            {
//                uint8_t __u6_addr8[16];   // 128 bit
//                #if defined __USE_MISC || defined __USE_GNU
//                uint16_t __u6_addr16[8];  // 64 bit
//                uint32_t __u6_addr32[4];  // 32 bit
//                #endif
//            } __in6_u;
//            #define s6_addr         __in6_u.__u6_addr8
//            #if defined __USE_MISC || defined __USE_GNU
//            # define s6_addr16      __in6_u.__u6_addr16
//            # define s6_addr32      __in6_u.__u6_addr32
//            #endif
//        };

        in6_addr ip = loopback_only ? in6addr_loopback : in6addr_any;
        addr6_.sin6_addr = ip;
        addr6_.sin6_port = sockets::HostToNetwork16(port);
    }
    else // ipv4
    {
        bzero(&addr_, sizeof(addr_));
        addr_.sin_family = AF_INET;
        in_addr_t ip = loopback_only ? kInAddrLoopback : kInAddrAny;
        addr_.sin_addr.s_addr = sockets::HostToNetwork32(ip);
        addr_.sin_port = sockets::HostToNetwork16(port);
    }
}

InetAddress::InetAddress(std::string_view ip, uint16_t port, bool ipv6)
{
    if (ipv6)
    {
        bzero(&addr6_, sizeof(addr6_));
        sockets::FromIpPort(ip.data(), port, &addr6_);
    }
    else
    {
        bzero(&addr_, sizeof(addr_));
        sockets::FromIpPort(ip.data(), port, &addr_);
    }
}

std::string InetAddress::ToIpPort() const
{
    char buf[64] = "";
    sockets::ToIpPort(buf, sizeof(buf), GetSockAddr());
    return buf;
}

std::string InetAddress::ToIp() const
{
    char buf[64] = "";
    sockets::ToIp(buf, sizeof(buf), GetSockAddr());
    return buf;
}

uint16_t InetAddress::ToPort() const
{
    return sockets::HostToNetwork16(PortNetEndian());
}

uint32_t InetAddress::IpNetEndian() const
{
    assert(family() == AF_INET);
    return addr_.sin_addr.s_addr;
}

static thread_local char t_resolve_buffer[64 * 1024];

bool InetAddress::Resolve(std::string_view hostname, InetAddress* address)
{
    assert(address);
    struct hostent hent;
    struct hostent* he = nullptr;
    int herrno = 0;
    bzero(&hent, sizeof(hent));
    int ret = gethostbyname_r(hostname.data(), &hent, t_resolve_buffer, sizeof(t_resolve_buffer), &he, &herrno);
    if (ret == 0 && he)
    {
        assert(he->h_addrtype == AF_INET && he->h_length == sizeof(uint32_t));
        address->addr_.sin_addr = *reinterpret_cast<struct in_addr*>(he->h_addr);
        return true;
    }
    else
    {
        if (ret != 0)
        {
            LOG_SYSERR << "InetAddress::Resolve";
        }
        return false;
    }
}

void InetAddress::SetScopeId(uint32_t scope_id)
{
    if (family() == AF_INET6)
    {
        addr6_.sin6_scope_id =scope_id;
    }
}

} // namespace net
} // namespace blaze

