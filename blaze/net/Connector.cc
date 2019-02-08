//
// Created by xi on 19-2-7.
//

#include <errno.h>

#include <blaze/log/Logging.h>

#include <blaze/net/EventLoop.h>
#include <blaze/net/Channel.h>
#include <blaze/net/SocketsOps.h>

#include <blaze/net/Connector.h>

namespace blaze
{
namespace net
{

const int Connector::kMaxRetryDelayMs = 30 * 1000; // 30 seconds
const int Connector::kInitRetryDelayMs = 500; // 0.5 second

Connector::Connector(EventLoop* loop, const InetAddress& server_addr) :
    loop_(loop),
    server_addr_(server_addr),
    connect_(false),
    state_(State::kDisconnected),
    retry_delay_ms_(kInitRetryDelayMs)
{
    LOG_DEBUG << "ctor[ " << this << "]";
}

Connector::~Connector()
{
    LOG_DEBUG << "Dtor[ " << this << "]";
    assert(!channel_);
}

void Connector::Start()
{
    connect_ = true;
    loop_->RunInLoop(std::bind(&Connector::StartInLoop, this)); // FIXME: unsafe
}

void Connector::StartInLoop()
{
    loop_->AssertInLoopThread();
    assert(state_ == State::kDisconnected);
    if (connect_)
    {
        Connect();
    }
    else
    {
        LOG_DEBUG << "Do not connect";
    }
}

void Connector::Stop()
{
    connect_ = false;
    // FIXME: cancel timer
    loop_->QueueInLoop(std::bind(&Connector::StopInLoop, this)); // FIXME: unsafe
}

void Connector::StopInLoop()
{
    loop_->AssertInLoopThread();
    if (state_ == State::kConnecting)
    {
        SetState(State::kDisconnected);
        int sockfd = RemoveAndResetChannel();
        Retry(sockfd);
    }
}

void Connector::Connect()
{
    int sockfd = sockets::CreateNonBlockingOrDie(server_addr_.family());
    int ret = sockets::connect(sockfd, server_addr_.GetSockAddr());
    int saved_errno = (ret == 0) ? 0 : errno;
    switch (saved_errno)
    {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            Connecting(sockfd);
            break;

        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            Retry(sockfd);
            break;

        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_SYSERR << "connect error in Connector::StartInLoop " << saved_errno;
            sockets::close(sockfd);
            break;

        default:
            LOG_SYSERR << "unexpected error in Connector::StartInLoop " << saved_errno;
            sockets::close(sockfd);
            // connection_error_callback_()
            break;
    }
}

void Connector::Restart()
{
    loop_->AssertInLoopThread();
    SetState(State::kDisconnected);
    retry_delay_ms_ = kInitRetryDelayMs;
    connect_ = true;
    StartInLoop();
}

void Connector::Connecting(int sockfd)
{
    SetState(State::kConnecting);
    assert(!channel_);
    channel_.reset(new Channel(loop_, sockfd));
    channel_->SetWriteCallback(std::bind(&Connector::HandleWrite, this)); // FIXME: unsafe
    channel_->SetErrorCallback(std::bind(&Connector::HandleError, this)); // FIXME: unsafe
    // channel->Tie(shared_from_this()) is not working,
    // because channel_ is not managed by shared_ptr
    channel_->EnableWriting();
}

int Connector::RemoveAndResetChannel()
{
    channel_->DisableAll();
    channel_->Remove();
    int sockfd = channel_->fd();
    // Can't not reset channel_ here, because we are inside Channel::HandleEvent
    loop_->QueueInLoop(std::bind(&Connector::ResetChannel, this));
    return sockfd;
}

void Connector::ResetChannel()
{
    channel_.reset();
}

void Connector::HandleWrite()
{
    LOG_TRACE << "Connector::HandleWrite state=" << ToUType(state_);
    if (state_ == State::kConnecting)
    {
        int sockfd = RemoveAndResetChannel();
        int err = sockets::GetSocketError(sockfd);
        if (err)
        {
            LOG_WARN << "Connector::HandleWrite - SO_ERROR = "
                     << err << " " << strerror_tl(err);
            Retry(sockfd);
        }
        else if (sockets::IsSelfConnect(sockfd))
        {
            LOG_WARN << "Connector::HandleWrite - Self Connect";
            Retry(sockfd);
        }
        else
        {
            SetState(State::kConnected);
            if (connect_)
            {
                new_connection_callback_(sockfd);
            }
            {
                sockets::close(sockfd);
            }
        }
    }
    else
    {
        // what happened ?
        assert(state_ == State::kDisconnected);
    }
}

void Connector::HandleError()
{
    LOG_ERROR << "Connector::HandleError state=" << ToUType(state_);
    if (state_ == State::kConnecting)
    {
        int sockfd = RemoveAndResetChannel();
        int err = sockets::GetSocketError(sockfd);
        LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
        Retry(sockfd);
    }
}

void Connector::Retry(int sockfd)
{
    sockets::close(sockfd);
    SetState(State::kDisconnected);
    if (connect_)
    {
        LOG_INFO << "Connector::Retry() - Retry connecting to " << server_addr_.ToIpPort()
                 << " in " << retry_delay_ms_ << " milliseconds.";
        loop_->RunAfter(retry_delay_ms_ / 1000.0,
                        std::bind(&Connector::StartInLoop, shared_from_this()));
        retry_delay_ms_ = std::min(retry_delay_ms_ * 2, kMaxRetryDelayMs);
    }
    else
    {
        LOG_DEBUG << "Do not connect";
    }
}

}
}