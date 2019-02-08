//
// Created by xi on 19-2-7.
//

#ifndef BLAZE_TCPCLIENT_H
#define BLAZE_TCPCLIENT_H

#include <string_view>
#include <mutex>
#include <memory>

#include <blaze/net/Callbacks.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{
namespace net
{

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;
class EventLoop;
class InetAddress;

class TcpClient : public noncopyable
{
public:
    TcpClient(EventLoop* loop, const InetAddress& server_addr, const std::string_view& name);

    ~TcpClient(); // force out-line dtor, for std::unique_ptr member

    void Connect();

    void Disconnect();

    void Stop();

    TcpConnectionPtr connection() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    EventLoop* GetLoop() const
    {
        return loop_;
    }

    bool retry() const
    {
        return retry_;
    }

    void EnableRetry()
    {
        retry_ = true;
    }

    const std::string& name() const
    {
        return name_;
    }

    // Not thread safe
    void SetConnectionCallback(ConnectionCallback cb)
    {
        connection_callback_ = std::move(cb);
    }

    // Not thread safe
    void SetMessageCallback(MessageCallback cb)
    {
        message_callback_ = std::move(cb);
    }

    // Not thread safe
    void SetWriteCompleteCallback(WriteCompleteCallback cb)
    {
        write_complete_callback_ = std::move(cb);
    }

private:
    // Not thread safe, but in loop
    void NewConnection(int sockfd);

    // Not thread safe, but in loop
    void RemoveConnection(const TcpConnectionPtr& conn);

private:
    EventLoop* loop_;
    ConnectorPtr connector_;
    const std::string name_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    bool retry_;
    bool connect_;
    // always in loop thread
    int next_conn_id_;
    mutable std::mutex mutex_;
    TcpConnectionPtr connection_; // @GuardedBy(mutex_)
};

}
}

#endif //BLAZE_TCPCLIENT_H
