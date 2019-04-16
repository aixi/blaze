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

#include "DaytimeServer.h"


using namespace blaze;
using namespace blaze::net;

DaytimeServer::DaytimeServer(EventLoop* loop, const InetAddress& server_addr) :
    loop_(loop),
    server_(loop_, server_addr, "EchoServer")
{
    server_.SetConnectionCallback(std::bind(&DaytimeServer::OnConnection, this, _1));
}

void DaytimeServer::OnConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "EchoServer - " << conn->GetPeerAddress().ToIpPort()
             << " -> " << conn->GetLocalAddress().ToIpPort() << " is "
             << (conn->Connected() ? "UP" : "DOWN");
    if (conn->Connected())
    {
        conn->Send(Timestamp::Now().ToFormattedString() + '\n');
        conn->Shutdown();
    }
}

int main()
{
    LOG_INFO << "pid = " << ::getpid();
    EventLoop loop;
    InetAddress server_addr(2006);
    DaytimeServer server(&loop, server_addr);
    server.Start();
    loop.Loop();
}