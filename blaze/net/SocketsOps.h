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

ssize_t read(int sockfd, void* buf, size_t count);
ssize_t readv(int sockfd, const struct iovec* vec, int iovcnt);
size_t write(int sockfd, const void* buf, size_t count);

// create non-blocking socket file descriptor
// abort if any error

int createNonBlockingOrDie(sa_family_t family);

int bindOrDie(int sockfd, const struct sockaddr* addr);
int listenOrDie(int sockfd);

int accept(int sockfd, struct sockaddr_in6* addr);

int connect(int sockfd, const struct sockaddr* addr);

void close(int sockfd);

void shutdownWrite(int sockfd);

const struct sockaddr* sockaddr_cast(const struct sockaddr_in* addr);
const struct sockaddr* sockaddr_cast(const struct sockaddr_in6* addr);
struct sockaddr* sockaddr_cast(struct sockaddr_in6* addr);
const struct sockaddr_in* sockaddr_in_cast(const struct sockaddr* addr);
const struct sockaddr_in6* sockaddr_in6_cast(const struct sockaddr* addr);

struct sockaddr_in6 getLocalAddr(int sockfd);
struct sockaddr_in6 getPeerAddr(int sockfd);
bool isSelfConnect(int sockfd);

void toIpPort(char* buf, size_t size, const struct sockaddr* addr);
void toIp(char* buf, size_t size, const struct sockaddr* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in* addr);
void fromIpPort(const char* ip, uint16_t port, struct sockaddr_in6* addr);
void getSocketError(int sockfd);



} // namespace sockets
} // namespace net
} // namespace blaze

#endif //BLAZE_SOCKETSOPS_H
