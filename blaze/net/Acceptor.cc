//
// Created by xi on 19-2-3.
//


#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <blaze/log/Logging.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/InetAddress.h>
#include <blaze/net/SocketsOps.h>
#include <blaze/net/Acceptor.h>

namespace blaze
{

namespace net
{

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port) :
    loop_(loop),
    listen_socket_(sockets::CreateNonBlockingOrDie(listen_addr.family())),
    listen_channel_(loop_, listen_socket_.fd()),
    listening_(false),
    idle_fd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idle_fd_ >= 0);
    listen_socket_.SetReuseAddr(true);
    listen_socket_.SetReuseAddr(reuse_port);
    listen_socket_.bindAddress(listen_addr);
    // FIXME: where is the Timestamp receive_time parameter ?
    listen_channel_.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor()
{
    listen_channel_.DisableAll();
    ::close(idle_fd_);
}

void Acceptor::Listen()
{
    loop_->AssertInLoopThread();
    listening_ = true;
    listen_socket_.listen();
    listen_channel_.EnableReading();
}

void Acceptor::HandleRead()
{
    loop_->AssertInLoopThread();
    InetAddress peer_addr;
    int connfd = listen_socket_.accept(&peer_addr);
    if (connfd >= 0)
    {
        LOG_TRACE << "Accepts of " << peer_addr.ToIpPort().c_str();
        if (new_connection_callback_)
        {
            new_connection_callback_(connfd, peer_addr);
        }
        else 
        {
            ::close(connfd);
        }
    }
    else 
    {
        LOG_SYSERR << "in Acceptor::HandleRead";
        if (errno == EMFILE) // too many opened file, use ulimit -n to change a process's maximum open files number
        {
            ::close(idle_fd_);
            idle_fd_ = ::accept(listen_socket_.fd(), nullptr, nullptr);
            ::close(idle_fd_);
            idle_fd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

}

}
