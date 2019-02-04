//
// Created by xi on 19-2-4.
//
#include <blaze/log/Logging.h>
#include <blaze/net/SocketsOps.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/Socket.h>
#include <blaze/net/Channel.h>
#include <blaze/net/TcpConnection.h>

namespace blaze
{
namespace net
{



TcpConnection::TcpConnection(EventLoop* loop,
                             std::string_view name,
                             int connfd,
                             const InetAddress& local_addr,
                             const InetAddress& peer_addr) :
    loop_(loop),
    name_(name),
    state_(ConnState::kConnecting),
    socket_(new Socket(connfd)),
    channel_(new Channel(loop_, connfd)),
    local_addr_(local_addr),
    peer_addr_(peer_addr)
{
    channel_->SetReadCallback([this]{HandleRead();});
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at" << this
              << " fd=" << connfd;
    socket_->SetKeepAlive(true);
    channel_->EnableReading();
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at" << this
              << " fd=" << channel_->fd()
              << " state=" << StateToCStr();
}

void TcpConnection::ConnectionEstablished()
{
    loop_->AssertInLoopThread();
    assert(state_ == ConnState::kConnecting);
    SetState(ConnState::kConnected);
    connection_callback_(shared_from_this());
}

void TcpConnection::ConnectionDestroyed()
{
    loop_->AssertInLoopThread();
    assert(state_ == ConnState::kConnected);
    SetState(ConnState::kDisconnected);
    channel_->DisableAll();
    connection_callback_(shared_from_this()); // ????
    channel_->Remove();
}

void TcpConnection::HandleRead()
{
    int saved_errno;
    ssize_t n = input_buffer_.ReadFd(channel_->fd(), &saved_errno);
    if (n > 0)
    {
        message_callback_(shared_from_this(), &input_buffer_, Timestamp::Now());
    }
    else if (n == 0)
    {
        HandleClose();
    }
    else
    {
        errno = saved_errno;
        LOG_SYSERR << "TcpConnection::HandleRead";
        HandleError();
    }
}

void TcpConnection::HandleWrite()
{
    // TODO
}

void TcpConnection::HandleClose()
{
    loop_->AssertInLoopThread();
    LOG_TRACE << "TcpConnection::HandleClose state = " << StateToCStr();
    assert(state_ == ConnState::kConnected);
    // we don't close fd, leave it to dtor, so we can find leaks easily
    channel_->DisableAll();
    close_callback_(shared_from_this());
}

void TcpConnection::HandleError()
{
    int err = sockets::GetSocketError(channel_->fd());
    LOG_ERROR << "TcpConnection::HandleError [" << name_.c_str()
              << "] - SO_ERROR=" << err << " " << strerror_tl(err);
}

const char* TcpConnection::StateToCStr() const
{
    switch (state_)
    {
        case ConnState::kConnecting:
            return "kConnecting";
        case ConnState::kConnected:
            return "kConnected";
        default:
            return "unknown state";
    }
}

}
}
