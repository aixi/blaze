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
              << " state=" << StateToString();
}

void TcpConnection::ConnectEstablished()
{
    loop_->AssertInLoopThread();
    assert(state_ == ConnState::kConnecting);
    SetState(ConnState::kConnected);
    connection_callback_(shared_from_this());
}

void TcpConnection::HandleRead()
{
    int saved_errno;
    input_buffer_.ReadFd(channel_->fd(), &saved_errno);
    message_callback_(shared_from_this(), &input_buffer_, Timestamp::Now());
}

const char* TcpConnection::StateToString() const
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
