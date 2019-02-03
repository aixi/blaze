//
// Created by xi on 19-1-29.
//

#ifndef BLAZE_SOCKETSOPS_H
#define BLAZE_SOCKETSOPS_H

#include <arpa/inet.h>

namespace blaze
{
namespace net
{
namespace sockets
{

enum class ByteOrder
{
    kLittleEndian,
    kBigEndian
};

// create non-blocking socket file descriptor
// abort if any error

int CreateNonBlockingOrDie(sa_family_t family);

int bindOrDie(int sockfd, const struct sockaddr* addr);
int listenOrDie(int sockfd);

int accept(int sockfd, struct sockaddr_in6* /*out*/ addr);

int connect(int sockfd, const struct sockaddr* addr);

void close(int sockfd);

void shutdownWrite(int sockfd);

ssize_t read(int sockfd, void* buf, size_t count);
ssize_t readv(int sockfd, const struct iovec* vec, int iovcnt);
ssize_t write(int sockfd, const void* buf, size_t count);

// to sockaddr
const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);

// to sockaddr_in
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

struct sockaddr_in6 GetLocalAddr(int sockfd);
struct sockaddr_in6 GetPeerAddr(int sockfd);

void ToIpPort(char* /*out*/ buf, size_t size, const struct sockaddr* addr);
void ToIp(char* /*out*/ buf, size_t size, const struct sockaddr* addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in* /*out*/ addr);
void FromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* /*out*/ addr);

int GetSocketError(int sockfd);
bool IsSelfConnect(int sockfd);

ByteOrder GetByteOrder();

} // namespace sockets
} // namespace net
} // namespace blaze

#endif //BLAZE_SOCKETSOPS_H
