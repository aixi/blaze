//
// Created by xi on 19-2-4.
//

#ifndef BLAZE_TCPCONNECTION_H
#define BLAZE_TCPCONNECTION_H

#include <string_view>
#include <memory>
#include <any>

#include <blaze/net/InetAddress.h>
#include <blaze/net/Callbacks.h>
#include <blaze/net/Buffer.h>
#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Types.h>

// included in <netinet/tcp.h>
struct tcp_info;

namespace blaze
{

namespace net
{

class EventLoop;
class Socket;
class Channel;

class TcpConnection : public noncopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    // User shall not explicitly create this object

    TcpConnection(EventLoop* loop,
                  std::string_view name,
                  int connfd,
                  const InetAddress& local_addr,
                  const InetAddress& peer_addr);


    ~TcpConnection();

    void SetConnectionCallback(const ConnectionCallback& cb)
    {
        connection_callback_ = cb;
    }

    void SetMessageCallback(const MessageCallback& cb)
    {
        message_callback_ = cb;
    }

    // internal use only
    void SetCloseCallback(const CloseCallback& cb)
    {
        close_callback_ = cb;
    }

    void SetHighwaterMarkCallback(const HighWaterMarkCallback& cb, size_t highwater_mark)
    {
        highwater_mark_callback_ = cb;
        highwater_mark_ = highwater_mark;
    }

    void SetWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        write_complete_callback_ = cb;
    }

    // Nagle's algorithm
    void SetTcpNoDelay(bool on);

    void SetKeepAlive(bool on);

    void StartRead();
    void StopRead();
    bool IsReading() const // NOT thread safe, may race with Start/StopReadInLoop
    {
        return reading_;
    }

    bool GetTcpInfo(struct tcp_info* tcpi) const;

    std::string GetTcpInfoString() const;

    void SetContext(const std::any& context)
    {
        context_ = context;
    }

    const std::any& GetContext() const
    {
        return context_;
    }

    std::any& GetContext()
    {
        return context_;
    }

    // called when TcpServer accepts a new connection
    // should be called only once
    void ConnectionEstablished();

    // called when TcpServer has removed me from its connectionMap
    // shall be called only once
    // NOTE: someone may call ConnectionDestroyed() before HandleClose()
    void ConnectionDestroyed();

    void Send(const void* data, size_t len);

    void Send(const std::string_view& message);

    void Send(Buffer* buf);

    void Shutdown();

    void ForceClose();

    void ForceCloseWithDelay(double seconds);

    bool connected() const
    {
        return State::kConnected == state_;
    }

    bool disconnected() const
    {
        return State::kDisconnected == state_;
    }

    EventLoop* GetLoop() const
    {
        return loop_;
    }

    const std::string& name() const
    {
        return name_;
    }

    const InetAddress& GetLocalAddress() const
    {
        return local_addr_;
    }

    const InetAddress& GetPeerAddress() const
    {
        return peer_addr_;
    }

    // advance interface

    Buffer* GetInputBuffer()
    {
        return &input_buffer_;
    }

    Buffer* GetOutputBuffer()
    {
        return &output_buffer_;
    }

private:
    enum class State
    {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected
    };

    void HandleRead(Timestamp when);
    void HandleWrite();
    void HandleClose();
    void HandleError();

    void SendInLoop(const std::string_view& message);
    void SendInLoop(const void* data, size_t len);
    void ShutDownInLoop();

    void ForceCloseInLoop();

    void StartReadInLoop();
    void StopReadInLoop();

    void SetState(State state)
    {
        state_ = state;
    }

    const char* StateToCStr() const;

private:
    EventLoop* loop_;
    const std::string name_;
    size_t highwater_mark_;
    bool reading_;
    State state_; // FiXME: use atomic variable
    // Do not expose Channel.h Socket.h in public header
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress local_addr_;
    InetAddress peer_addr_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;
    HighWaterMarkCallback highwater_mark_callback_;
    WriteCompleteCallback write_complete_callback_;
    Buffer input_buffer_;
    Buffer output_buffer_; // FIXME: use std::list<Buffer> and ::writev()
    std::any context_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace net

} // namespace blaze

#endif //BLAZE_TCPCONNECTION_H
