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
    channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, _1));
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

void TcpConnection::Send(const std::string& message)
{
    if (state_ == ConnState::kConnected)
    {
        if (loop_->IsInLoopThread())
        {
            SendInLoop(message);
        }
        else
        {
            loop_->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, message));
        }
    }
}

void TcpConnection::Shutdown()
{
    // FIXME: use compare_and_swap
    if (state_ == ConnState::kDisconnected)
    {
        SetState(ConnState::kDisconnecting);
        // FIXME: shared_from_this() ?
        loop_->RunInLoop([this]{ShutDownInLoop();});
    }
}

void TcpConnection::SendInLoop(const std::string& message)
{
    loop_->AssertInLoopThread();
    ssize_t has_written = 0;
    // if no thing in output_buffer_, write directly
    if (!channel_->IsWriting() && output_buffer_.ReadableBytes() == 0)
    {
        has_written = sockets::write(channel_->fd(), message.data(), message.size());
        if (has_written >= 0)
        {
            if (implicit_cast<size_t>(has_written) < message.size())
            {
                LOG_TRACE << "I am going to write more data";

            }
        }
        else
        {
            has_written = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::SendInLoop";
            }
        }
    }

    // let channel_ observe writable event
    // TcpConnection::HandleWrite will write the remaining
    // message in output_buffer_
    assert(has_written >= 0);
    if (implicit_cast<size_t>(has_written) < message.size())
    {
        output_buffer_.Append(message.data() + has_written, message.size() - has_written);
        if (!channel_->IsWriting())
        {
            channel_->EnableWriting();
        }
    }
}

void TcpConnection::ShutDownInLoop()
{
    loop_->AssertInLoopThread();
    if (!channel_->IsWriting())
    {
        socket_->shutdownWrite();
    }
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
    if (state_ == ConnState::kConnected)
    {
        SetState(ConnState::kDisconnected);
        channel_->DisableAll();
        connection_callback_(shared_from_this());
    }
    channel_->Remove();
}

void TcpConnection::HandleRead(Timestamp when)
{
    int saved_errno;
    ssize_t n = input_buffer_.Readfd(channel_->fd(), &saved_errno);
    if (n > 0)
    {
        message_callback_(shared_from_this(), &input_buffer_, when);
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
    loop_->AssertInLoopThread();
    if (channel_->IsWriting())
    {
        ssize_t n = sockets::write(channel_->fd(),
                                   output_buffer_.BeginRead(),
                                   output_buffer_.ReadableBytes());
        if (n > 0)
        {
            output_buffer_.Retrieve(n);
            // all data in output_buffer_ has been write
            // Stop observe writable event, because we use level-trigger, avoid busy loop
            if (output_buffer_.ReadableBytes() == 0)
            {
                channel_->DisableWriting();
                if (state_ == ConnState::kDisconnecting)
                {
                    ShutDownInLoop();
                }
            }
            else
            {
                LOG_TRACE << "I am going to write more data";
            }
        }
        else
        {
            LOG_SYSERR << "TcpConnection::HandleWrite";
        }
    }
    else
    {
        LOG_TRACE << "Connection is down no more writing";
    }
}

void TcpConnection::HandleClose()
{
    loop_->AssertInLoopThread();
    LOG_TRACE << "TcpConnection::HandleClose state = " << StateToCStr();
    assert(state_ == ConnState::kConnected || state_ == ConnState::kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily
    SetState(ConnState::kDisconnected);
    channel_->DisableAll();
    TcpConnectionPtr guard_this(shared_from_this());
    connection_callback_(guard_this);
    close_callback_(guard_this);
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
        case ConnState::kDisconnecting:
            return "kDisconnecting";
        case ConnState::kDisconnected:
            return "kDisconnected";
        default:
            return "unknown state";
    }
}

}
}
