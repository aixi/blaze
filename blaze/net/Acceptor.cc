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
    idlefd_(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{
    assert(idlefd_ >= 0);
    listen_socket_.SetReuseAddr(true);
    listen_socket_.SetReuseAddr(reuse_port);
    listen_socket_.bindAddress(listen_addr);
    listen_channel_.SetReadCallback([this]{HandleRead();});
}

Acceptor::~Acceptor()
{
    listen_channel_.DisableAll();
    ::close(idlefd_);
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
        if (errno == EMFILE)
        {
            ::close(idlefd_);
            idlefd_ = ::accept(listen_socket_.fd(), nullptr, nullptr);
            ::close(idlefd_);
            idlefd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

}

}
