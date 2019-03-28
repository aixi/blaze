//
// Created by xi on 19-3-15.
//

#ifndef BLAZE_TCPCLIENT_H
#define BLAZE_TCPCLIENT_H

#include <mutex>
#include <blaze/utils/Types.h>
#include <blaze/net/TcpConnection.h>

namespace blaze
{
namespace net
{

class Connector;
typedef std::shared_ptr<Connector> ConnectorPtr;

class TcpClient
{
public:
    // TcpClient(EventLoop* loop);
    // TcpClient(EventLoop* loop, const string& host, uint16_t port);
    TcpClient(EventLoop* loop,
              const InetAddress& serverAddr,
              const std::string_view& nameArg);
    ~TcpClient();  // force out-line dtor, for std::unique_ptr members.

    void Connect();
    void Disconnect();
    void Stop();

    TcpConnectionPtr Connection() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    EventLoop* GetLoop() const { return loop_; }
    bool Retry() const { return retry_; }
    void EnableRetry() { retry_ = true; }

    const std::string& name() const
    { return name_; }

    /// Set connection callback.
    /// Not thread safe.
    void SetConnectionCallback(ConnectionCallback cb)
    { connection_callback_ = std::move(cb); }

    /// Set message callback.
    /// Not thread safe.
    void SetMessageCallback(MessageCallback cb)
    { message_callback_ = std::move(cb); }

    /// Set write complete callback.
    /// Not thread safe.
    void SetWriteCompleteCallback(WriteCompleteCallback cb)
    { write_complete_callback_ = std::move(cb); }

    DISABLE_COPY_AND_ASSIGN(TcpClient);

private:
    /// Not thread safe, but in loop
    void NewConnection(int sockfd);
    /// Not thread safe, but in loop
    void RemoveConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    ConnectorPtr connector_; // avoid revealing Connector
    const std::string name_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    bool retry_;   // atomic
    bool connect_; // atomic
    // always in loop thread
    int next_conn_id_;
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_; // GUARDED_BY(mutex_)
};

}  // namespace net
} // namespace muduo
#endif //BLAZE_TCPCLIENT_H
