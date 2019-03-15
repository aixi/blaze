//
// Created by xi on 19-1-29.
//

#include <errno.h>
#include <blaze/net/SocketsOps.h>
#include <blaze/net/Buffer.h>
#include <blaze/utils/Types.h>

using namespace blaze;
using namespace blaze::net;

const char Buffer::kCRLF[] = "\r\n";
const size_t Buffer::kCheapPrepend = 8;
const size_t Buffer::kInitialSize = 1024;

ssize_t Buffer::Readfd(int fd, int* saved_errno)
{
    char extra_buf[65536]; // 64KiB
    struct iovec vec[2];
    const size_t writable = WritableBytes();
    vec[0].iov_base = BeginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extra_buf;
    vec[1].iov_len = sizeof(extra_buf);

    // when there are enough writable bytes in Buffer, don't read data into extra_buf
    // when extra_buf is used, use 128KB - 1 bytes at most

    const int iovcnt = (writable < sizeof(extra_buf)) ? 2 : 1;
    const ssize_t n = sockets::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        *saved_errno = errno;
    }
    else if (implicit_cast<size_t>(n) <= writable)
    {
        write_index_ += n;
    }
    else
    {
        write_index_ = buffer_.size();
        Append(extra_buf, n - writable);
    }
    return n;
}
