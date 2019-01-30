//
// Created by xi on 19-1-29.
//

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h> // snprintf
#include <sys/uio.h> // readv

#include <blaze/log/Logging.h>
#include <blaze/utils/Types.h>
#include <blaze/net/Endian.h>
#include <blaze/net/SocketsOps.h>

using namespace blaze;
using namespace blaze::net;

namespace
{

using SA = struct sockaddr;

#if VALGRIND || defined(NO_ACCEPT4)

void setNonBlockingAndCloseOnExec(int sockfd)
{
    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    // FIXME: check ret

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
    // FIXME: check ret
    UnusedVariable(ret);
}

#endif

} // anonymous namespace


int sockets::createNonBlockingOrDie(sa_family_t family)
{
    // return socket fd
#if VALGRIND
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSERR << "socket::createNonblockingOrDie";
    }
    setNonBlockingAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        LOG_SYSERR << "socket::createNonblockingOrDie";
    }
#endif
    return sockfd;
}

int sockets::bindOrDie(int sockfd, const struct sockaddr* addr)
{
    int ret = ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
    if (ret < 0)
    {
        // FIXME: check errno EACCES ? EADDRINUSE ?
        LOG_SYSERR << "sockets::bindOrDie";
    }
    return ret;
}

int sockets::listenOrDie(int sockfd)
{
    int ret = ::listen(sockfd, SOMAXCONN);
    // 'echo 1000 >/proc/sys/net/core/somaxconn' to change SOMAXCONN, default 128
    // exceed backlog will trigger client errno ECONNREFUSED
    if (ret < 0)
    {
        LOG_SYSERR << "sockets::listenOrDie";
    }
    return ret;
}

int sockets::accept(int sockfd, struct sockaddr_in6* addr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(*addr));
#if VALGRIND || defined(NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockingAndCloseOnExec(sockfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                                   SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif
    if (connfd < 0)
    {
        int saved_errno = errno;
        LOG_SYSERR << "sockets::accept";
        switch (saved_errno)
        {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO: // ???
            case EPERM:
            case EMFILE: // maximum number of opened file descriptor per process ?
                // expect error
                errno = saved_errno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << saved_errno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << saved_errno;
                break;
        }
    }
    return connfd;
}

int sockets::connect(int sockfd, const struct sockaddr* addr)
{
    // return 0 if success, -1 if fail
    // FIXME: check errno, ECONNREFUSED ? ETIMEDOUT ?
    return ::connect(sockfd, addr, static_cast<socklen_t>(sizeof(struct sockaddr_in6)));
}


void sockets::close(int sockfd)
{
    // only ref count minus 1, if fork a child process it owns this socket it will not close
    if (::close(sockfd) < 0)
    {
        LOG_SYSERR << "sockets::close";
    }
}

void sockets::shutdownWrite(int sockfd)
{
    // shall not write on this socket fd
    // OS will send all data in kernel's output buffer before the socket fd has been actually closed
    if (::shutdown(sockfd, SHUT_WR) < 0)
    {
        LOG_SYSERR << "sockets::shutdownWrite";
    }
}

ssize_t sockets::read(int sockfd, void* buf, size_t count)
{
    return ::read(sockfd, buf, count);
}

ssize_t sockets::readv(int sockfd, const struct iovec* vec, int iovcnt)
{
    return ::readv(sockfd, vec, iovcnt);
}

ssize_t sockets::write(int sockfd, const void* buf, size_t count)
{
    return ::write(sockfd, buf, count);
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in* addr)
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));
}

const struct sockaddr* sockets::sockaddr_cast(const struct sockaddr_in6* addr)
{
    return static_cast<const struct sockaddr*>(implicit_cast<const void*>(addr));

}

struct sockaddr* sockets::sockaddr_cast(struct sockaddr_in6* addr)
{
    return static_cast<struct sockaddr*>(implicit_cast<void*>(addr));

}

const struct sockaddr_in* sockets::sockaddr_in_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in*>(implicit_cast<const void*>(addr));
}

const struct sockaddr_in6* sockets::sockaddr_in6_cast(const struct sockaddr* addr)
{
    return static_cast<const struct sockaddr_in6*>(implicit_cast<const void*>(addr));
}

void sockets::toIpPort(char* buf, size_t size, const struct sockaddr* addr)
{
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = sockets::NetworkToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}

void sockets::toIp(char* buf, size_t size, const struct sockaddr* addr)
{
    if (addr->sa_family == AF_INET)
    {
        assert(size >= INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf, static_cast<socklen_t>(size));
    }
    else if (addr->sa_family == AF_INET6)
    {
        assert(size >= INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, static_cast<socklen_t>(size));
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = sockets::HostToNetwork16(port);
    // inet_pton return 1 if success
    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
    {
        LOG_SYSERR << "sockets::fromIpPort";
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr)
{
    addr->sin6_family = AF_INET6;
    addr->sin6_port = sockets::HostToNetwork16(port);
    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
    {
        LOG_SYSERR << "sockets::fromIpPort";
    }
}

int sockets::getSocketError(int sockfd)
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));
    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

struct sockaddr_in6 sockets::getLocalAddr(int sockfd)
{
    struct sockaddr_in6 local_addr;
    bzero(&local_addr, sizeof(local_addr));
    socklen_t addr_len = static_cast<socklen_t>(sizeof(local_addr));
    if (::getsockname(sockfd, sockaddr_cast(&local_addr), &addr_len) < 0)
    {
        LOG_SYSERR << "sockets::getLocalAddr";
    }
    return local_addr;
}

struct sockaddr_in6 sockets::getPeerAddr(int sockfd)
{
    struct sockaddr_in6 peer_addr;
    bzero(&peer_addr, sizeof(peer_addr));
    socklen_t addr_len = static_cast<socklen_t>(sizeof(peer_addr));
    if (::getpeername(sockfd, sockaddr_cast(&peer_addr), &addr_len) < 0)
    {
        LOG_SYSERR << "sockets::getPeerAddr";
    }
    return peer_addr;
}

bool sockets::isSelfConnect(int sockfd)
{
    struct sockaddr_in6 local_addr = getLocalAddr(sockfd);
    struct sockaddr_in6 peer_addr = getPeerAddr(sockfd);
    if (local_addr.sin6_family == AF_INET)
    {
        const struct sockaddr_in* laddr4 = reinterpret_cast<struct sockaddr_in*>(&local_addr);
        const struct sockaddr_in* raddr4 = reinterpret_cast<struct sockaddr_in*>(&peer_addr);
        return laddr4->sin_port == raddr4->sin_port
            && laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;

    }
    else if (local_addr.sin6_family == AF_INET6)
    {
        return local_addr.sin6_port == peer_addr.sin6_port
            && memcmp(&local_addr.sin6_addr, &peer_addr.sin6_addr, sizeof(local_addr.sin6_addr)) == 0;
    }
    else
    {
        return false;
    }
}

sockets::ByteOrder GetByteOrder()
{
    union
    {
        short value;
        char bytes[sizeof(value)];
    } test;
    test.value = 0x0102;
    if (test.bytes[0] == 1 && test.bytes[1] == 2)
    {
        return sockets::ByteOrder::kBigEndian;
    }
    else
    {
        return sockets::ByteOrder::kLittleEndian;
    }
}
