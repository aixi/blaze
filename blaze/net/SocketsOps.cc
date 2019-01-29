//
// Created by xi on 19-1-29.
//

#include <sys/uio.h>
#include <blaze/net/SocketsOps.h>

using namespace blaze;
using namespace blaze::net;

ssize_t sockets::readv(int sockfd, const struct iovec* vec, int iovcnt)
{
    return ::readv(sockfd, vec, iovcnt);
}