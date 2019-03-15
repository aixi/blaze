//
// Created by xi on 19-2-7.
//

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <utility>

#include <blaze/log/Logging.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/TcpServer.h>

using namespace blaze;
using namespace blaze::net;

class EchoServer
{
public:
    EchoServer(EventLoop* loop, const InetAddress& listen_addr) :
        loop_(loop),
        server_(loop_, listen_addr, "EchoServer")
    {
        server_.SetConnectionCallback(std::bind(&EchoServer::OnConnection, this, _1));
        server_.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
    }

    void SetThreadNum(int thread_num)
    {
        server_.SetThreadNum(thread_num);
    }

    void Start()
    {
        server_.Start();
    }
private:
    void OnConnection(const TcpConnectionPtr& conn)
    {
        LOG_TRACE << conn->GetPeerAddress().ToIpPort() << "->"
                  << conn->GetLocalAddress().ToIpPort() << " is "
                  << (conn->Connected() ? "up" : "down");
        LOG_INFO << conn->GetTcpInfoString();
        conn->Send("hello\n");
    }

    void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp when)
    {
        std::string msg(buf->RetrieveAllAsString());
        LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << when.ToFormattedString();
        if (msg == "exit\n")
        {
            conn->Send("bye\n");
            conn->Shutdown();
        }
        if (msg == "quit\n")
        {
            loop_->Quit();
        }
        conn->Send(msg);
    }

private:
    EventLoop* loop_;
    TcpServer server_;
};

int main()
{
    LOG_INFO << "pid = " << getpid();
    LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
    EventLoop loop;
    InetAddress listen_addr(9981, false, false);
    EchoServer server(&loop, listen_addr);
    server.SetThreadNum(4);
    server.Start();
    loop.Loop();
    return 0;
}