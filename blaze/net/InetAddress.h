//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_INETADDRESS_H
#define BLAZE_INETADDRESS_H


#include <netinet/in.h>
#include <string_view>
#include <blaze/utils/copyable.h>

namespace blaze
{

namespace net
{

namespace sockets
{

const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr6);

}

// Wrapper of sockaddr_in
// This is an POD interface class

class InetAddress : public copyable
{

public:

    // Mostly used in TcpServer listening

    explicit InetAddress(uint16_t port = 0, bool loopback_only = false, bool ipv6 = false);

    // ip should be "1.2.3.4"

    InetAddress(std::string_view ip, uint16_t port, bool ipv6 = false);

    // Mostly used when accepting new connections
    explicit InetAddress(const struct sockaddr_in& addr) :
        addr_(addr)
    {}

    explicit InetAddress(const struct sockaddr_in6& addr6) :
        addr6_(addr6)
    {}

    sa_family_t family() const
    {
        return addr_.sin_family;
    }

    std::string ToIp() const;
    std::string ToIpPort() const;
    uint16_t ToPort() const;

    // default copy/assignment are OK

    const struct sockaddr* GetSockAddr() const
    {
        return sockets::sockaddr_cast(&addr6_);
    }

    void SetSockAddrIn6(const struct sockaddr_in6& addr6)
    {
        addr6_ = addr6;
    }

    uint32_t IpNetEndian() const;

    uint16_t PortNetEndian() const
    {
        return addr_.sin_port;
    }

    // Resolve hostname to IP address, not change port or sin_family
    // return true on success
    // thread safe
    // NOTE: it could block
    static bool Resolve(std::string_view hostname, InetAddress* /*out*/ address);

    // set IPv6 ScopeID
    void SetScopeId(uint32_t scope_id);

private:
    union
    {
        struct sockaddr_in addr_;
        struct sockaddr_in6 addr6_;
    };
};

}

}

#endif //BLAZE_INETADDRESS_H
