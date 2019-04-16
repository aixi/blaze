//
// Created by xi on 19-4-16.
//

#include <unistd.h>
#include <string>
#include <blaze/log/Logging.h>
#include <blaze/net/Buffer.h>
#include <blaze/net/InetAddress.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/EventLoop.h>

#include "echo_server.h"


using namespace blaze;
using namespace blaze::net;

EchoServer::EchoServer(EventLoop* loop, const InetAddress& server_addr) :
    loop_(loop),
    server_(loop_, server_addr, "EchoServer")
{
    server_.SetConnectionCallback(std::bind(&EchoServer::OnConnection, this, _1));
    server_.SetMessageCallback(std::bind(&EchoServer::OnMessage, this, _1, _2, _3));
}

void EchoServer::OnConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "EchoServer - " << conn->GetPeerAddress().ToIpPort()
             << " -> " << conn->GetLocalAddress().ToIpPort() << " is "
             << (conn->Connected() ? "UP" : "DOWN");
}

void EchoServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    std::string msg(buf->RetrieveAllAsString());
    LOG_INFO << conn->name() << " echo " << msg.size() << " bytes,"
             << " data received at " << time.ToString();
    conn->Send(msg);
}

int main()
{
    LOG_INFO << "pid = " << ::getpid();
    EventLoop loop;
    InetAddress server_addr(2007);
    EchoServer server(&loop, server_addr);
    server.Start();
    loop.Loop();
}