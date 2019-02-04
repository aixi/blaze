//
// Created by xi on 19-2-4.
//

#ifndef BLAZE_TCPCONNECTION_H
#define BLAZE_TCPCONNECTION_H

#include <string_view>
#include <memory>

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

    // called when TcpServer accepts a new connection
    // should be called only once
    void ConnectionEstablished();

    // called when TcpServer has removed me from its connectionMap
    // shall be called only once
    // NOTE: someone may call ConnectionDestroyed() before HandleClose()
    void ConnectionDestroyed();

    void Send(const std::string& message);

    void Shutdown();

    bool Connected() const
    {
        return ConnState::kConnected == state_;
    }

    bool Disconnected() const
    {
        return ConnState::kDisconnected == state_;
    }

    const std::string& Name() const
    {
        return name_;
    }

    const InetAddress& LocalAddress() const
    {
        return local_addr_;
    }

    const InetAddress& PeerAddress() const
    {
        return peer_addr_;
    }



private:
    enum class ConnState
    {
        kConnecting,
        kConnected,
        kDisconnecting,
        kDisconnected
    };

    void SetState(ConnState state)
    {
        state_ = state;
    }

    void HandleRead(Timestamp when);
    void HandleWrite();
    void HandleClose();
    void HandleError();

    void SendInLoop(const std::string& message);
    void ShutDownInLoop();

    const char* StateToCStr() const;

private:
    EventLoop* loop_;
    const std::string name_;
    ConnState state_; // FiXME: use atomic variable
    // Do not expose Channel.h Socket.h in public header
    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;
    InetAddress local_addr_;
    InetAddress peer_addr_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;
    Buffer input_buffer_;
    Buffer output_buffer_;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace net

} // namespace blaze

#endif //BLAZE_TCPCONNECTION_H
