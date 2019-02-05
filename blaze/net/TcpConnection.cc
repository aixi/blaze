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

// in Callbacks.h

void DefaultConnectionCallback(const TcpConnectionPtr& conn)
{
    LOG_TRACE << conn->LocalAddress().ToIpPort() << " ->"
              << conn->PeerAddress().ToIpPort() << " is "
              << (conn->Connected() ? "up" : "down");
    // Do not call conn->ForceClose(), some users only want to register message callback only
}

void DefaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receive_time)
{
    buffer->RetrieveAll();
}


TcpConnection::TcpConnection(EventLoop* loop,
                             std::string_view name,
                             int connfd,
                             const InetAddress& local_addr,
                             const InetAddress& peer_addr) :
    loop_(loop),
    name_(name),
    highwater_mark_(64 * 1024 * 1024), // 64 MiB
    reading_(true),
    state_(ConnState::kConnecting),
    socket_(new Socket(connfd)),
    channel_(new Channel(loop_, connfd)),
    local_addr_(local_addr),
    peer_addr_(peer_addr)
{
    channel_->SetReadCallback(std::bind(&TcpConnection::HandleRead, this, _1));
    channel_->SetWriteCallback([this]{HandleWrite();});
    channel_->SetCloseCallback([this]{HandleClose();});
    channel_->SetErrorCallback([this]{HandleError();});
    LOG_DEBUG << "TcpConnection::ctor[" << name_ << "] at" << this
              << " fd=" << connfd;
    socket_->SetKeepAlive(true);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" << name_ << "] at" << this
              << " fd=" << channel_->fd()
              << " state=" << StateToCStr();
}

void TcpConnection::SetTcpNoDelay(bool on)
{
    socket_->SetTcpNoDelay(on);
}

void TcpConnection::SetKeepAlive(bool on)
{
    socket_->SetKeepAlive(on);
}

void TcpConnection::StartRead()
{
    loop_->RunInLoop(std::bind(&TcpConnection::StartReadInLoop, this));
}

void TcpConnection::StartReadInLoop()
{
    loop_->AssertInLoopThread();
    if (!reading_ || !channel_->IsReading())
    {
        channel_->EnableReading();
        reading_ = true;
    }
}

void TcpConnection::StopRead()
{
    loop_->RunInLoop(std::bind(&TcpConnection::StopReadInLoop, this));
}

void TcpConnection::StopReadInLoop()
{
    loop_->AssertInLoopThread();
    if (reading_ || channel_->IsReading())
    {
        channel_->DisableReading();
        reading_ = false;
    }
}

bool TcpConnection::GetTcpInfo(struct tcp_info* tcpi) const
{
    return socket_->GetTcpInfo(tcpi);
}

std::string TcpConnection::GetTcpInfoString() const
{
    char buf[1024];
    buf[0] = '\0';
    socket_->GetTcpInfoString(buf, sizeof(buf));
    return buf;
}

void TcpConnection::Send(const void* data, size_t len)
{
    SendInLoop(std::string_view(static_cast<const char*>(data), len));
}

void TcpConnection::Send(const std::string_view& message)
{
    if (state_ == ConnState::kConnected)
    {
        if (loop_->IsInLoopThread())
        {
            SendInLoop(message);
        }
        else
        {
            // FIXME: shared_from_this()
            // FIXME: avoid copy message
            loop_->RunInLoop([this, &message]{SendInLoop(std::string(message));});
        }
    }
}

void TcpConnection::Send(Buffer* buffer)
{
    if (state_ == ConnState::kConnected)
    {
        if (loop_->IsInLoopThread())
        {
            SendInLoop(buffer->BeginRead(), buffer->ReadableBytes());
        }
        else
        {
            // FIXME: shared_from_this()
            // FIXME: avoid copy message
            loop_->RunInLoop([this, buffer]{SendInLoop(buffer->RetrieveAllAsString());});
        }
    }
}

void TcpConnection::SendInLoop(const std::string_view& message)
{
    SendInLoop(message.data(), message.size());
}


void TcpConnection::SendInLoop(const void* data, size_t len)
{
    loop_->AssertInLoopThread();
    ssize_t has_written = 0;
    size_t remaining = len;
    bool fault_error = false;
    if (state_ == ConnState::kDisconnected)
    {
        LOG_WARN << "disconnected, give up writing";
    }
    // if no thing in output_buffer_, write directly
    if (!channel_->IsWriting() && output_buffer_.ReadableBytes() == 0)
    {
        // NOTE: call ::write only once, not call ::write repeatedly until it returns EAGAIN
        // If you fail to ::write all data for the first time, you will definitely return EAGAIN for the second time.
        has_written = sockets::write(channel_->fd(), data, len);
        if (has_written >= 0)
        {
            remaining -= has_written;
            if (implicit_cast<size_t>(has_written) < len)
            {
                LOG_TRACE << "I am going to write more data";

            }
            else if (write_complete_callback_)
            {
                loop_->QueueInLoop(std::bind(write_complete_callback_, shared_from_this()));
            }
        }
        else // ::write return value < 0, maybe error
        {
            has_written = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERR << "TcpConnection::SendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) // FIXME: any others
                {
                    fault_error = true;
                }
            }
        }
    }

    // let channel_ observe writable event
    // TcpConnection::HandleWrite will write the remaining data message in output_buffer_
    assert(remaining <= len);
    if (!fault_error && remaining > 0)
    {
        size_t old_len = output_buffer_.ReadableBytes();
        if (old_len + remaining >= highwater_mark_
            && old_len < highwater_mark_ // trigger only once, so check last time output_buffer's remaining data len
            && highwater_mark_callback_)
        {
            loop_->QueueInLoop(std::bind(highwater_mark_callback_, shared_from_this(), old_len + remaining));
        }
        output_buffer_.Append(static_cast<const char*>(data) + has_written, remaining);
        if (!channel_->IsWriting())
        {
            channel_->EnableWriting();
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
    channel_->Tie(shared_from_this());
    channel_->EnableReading();
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
        // NOTE: call ::write only once, not call ::write repeatedly until it returns EAGAIN
        // If you fail to ::write all data for the first time, you will definitely return EAGAIN for the second time.
        ssize_t n = sockets::write(channel_->fd(),
                                   output_buffer_.BeginRead(),
                                   output_buffer_.ReadableBytes());
        if (n > 0)
        {
            output_buffer_.Retrieve(n);
            // if all data in output_buffer_ has been written
            // Stop observe writable event, because we use level-trigger, avoid busy loop
            if (output_buffer_.ReadableBytes() == 0)
            {
                channel_->DisableWriting();
                if (write_complete_callback_)
                {
                    loop_->QueueInLoop(std::bind(write_complete_callback_, shared_from_this()));
                }
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
