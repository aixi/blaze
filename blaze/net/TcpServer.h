//
// Created by xi on 19-2-4.
//

#ifndef BLAZE_TCPSERVER_H
#define BLAZE_TCPSERVER_H

#include <string>
#include <memory>
#include <map>
#include <string_view>
#include <blaze/net/Callbacks.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{
namespace net
{

class EventLoop;
class Acceptor;
class InetAddress;

class TcpServer : public noncopyable
{
public:
    TcpServer(EventLoop* loop, const InetAddress& listen_addr, std::string_view name);

    ~TcpServer(); // forced out-of-line dtor, for std::unique_ptr member

    // It's harmless to call it multiple times
    // Thread safe
    void Start();

    // NOT thread safe
    void SetConnectionCallback(const ConnectionCallback& cb)
    {
        connection_callback_ = cb;
    }

    // NOT thread safe
    void SetMessageCallback(const MessageCallback& cb)
    {
        message_callback_ = cb;
    }

private:
    // Not thread safe, but in loop
    void NewConnection(int connfd, const InetAddress& peer_addr);

private:
    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_; // avoid exposing Acceptor.h in public header
    bool started_;
    int next_conn_id_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    ConnectionMap connections_;
};

} // namespace net
} // namespace blaze

#endif //BLAZE_TCPSERVER_H
