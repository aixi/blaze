//
// Created by xi on 19-3-15.
//

#include "Codec.h"

#include <unistd.h>
#include <stdio.h>
#include <set>
#include <mutex>
#include <blaze/log/Logging.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/TcpServer.h>

using namespace blaze;
using namespace blaze::net;

class ChatServer : public noncopyable
{
public:
    ChatServer(EventLoop* loop, const InetAddress& listen_addr) :
        server_(loop, listen_addr, "ChatServer"),
        codec_(std::bind(&ChatServer::OnStringMessage, this, _1, _2, _3))
    {
        server_.SetConnectionCallback(std::bind(&ChatServer::OnConnection, this, _1));
        server_.SetMessageCallback(std::bind(&LengthHeaderCodec::OnMessage, &codec_, _1, _2, _3));
    }

    void Start()
    {
        server_.Start();
    }

private:
    void OnConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->GetLocalAddress().ToIpPort() << " -> "
                 << conn->GetPeerAddress().ToIpPort() << " is "
                 << (conn->Connected() ? "UP" : "DOWN");
        if (conn->Connected())
        {
            connections_.insert(conn);
        }
        else
        {
            connections_.erase(conn);
        }
    }

    void OnStringMessage(const TcpConnectionPtr&, const std::string& message, Timestamp)
    {
        for (auto& conn : connections_)
        {
            codec_.Send(conn, message);
        }
    }

    using ConnectionList = std::set<TcpConnectionPtr>;
    TcpServer server_;
    LengthHeaderCodec codec_;
    ConnectionList connections_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 1)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress server_addr(port);
        ChatServer server(&loop, server_addr);
        server.Start();
        loop.Loop();
    }
    else
    {
        printf("Usage: %s port", argv[0]);
    }
}