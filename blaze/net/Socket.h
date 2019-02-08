//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_SOCKET_H
#define BLAZE_SOCKET_H

#include <blaze/utils/noncopyable.h>

// struct tcp_info is in <netinet/tcp.h>

struct tcp_info;

namespace blaze
{

namespace net
{

class InetAddress; // forward declare

// RAII handle of socket file descriptor
// It close the sockfd when destructs
// It's thread safe, all operations delegate to OS

class Socket : public noncopyable
{
public:
    explicit Socket(int sockfd) :
        sockfd_(sockfd)
    {}

    Socket(Socket&& rhs) noexcept;

    ~Socket();

    int fd() const
    {
        return sockfd_;
    }

    bool GetTcpInfo(struct tcp_info* info) const;

    bool GetTcpInfoString(char* buf, int len) const;

    // abort if address in use
    void bindAddress(const InetAddress& local_addr);

    // abort if address in use
    void listen();

    // return a non-negative int that is a descriptor for the accepted socket,
    // which has been set to non-blocking and close-on-exec, *peer_addr is assigned.
    // On error, return -1, and *peer_addr is untouched

    int accept(InetAddress* peer_addr);

    void shutdownWrite();

    // TCP_NODELAY, Nagle's algorithm
    void SetTcpNoDelay(bool on);

    // SO_REUSEADDR
    void SetReuseAddr(bool on);

    // SO_REUSEPORT
    void SetReusePort(bool on);

    // SO_KEEPALIVE
    void SetKeepAlive(bool on);

private:
    int sockfd_;
};

}

}

#endif //BLAZE_SOCKET_H
