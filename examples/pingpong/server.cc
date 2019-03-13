//
// Created by xi on 19-3-13.
//
#include <stdio.h>
#include <unistd.h>
#include <utility>
#include <sstream>
#include <blaze/log/Logging.h>
#include <blaze/net/InetAddress.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/TcpServer.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/Callbacks.h>

using namespace blaze;
using namespace blaze::net;

void OnConnection(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        conn->SetTcpNoDelay(true);
    }
}

void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
{
    conn->Send(buf);
}

int main(int argc, char* argv[])
{
    if (argc < 4)
    {
        printf("Usage: server <address> <port> <threads_num>\n");
        return 0;
    }
    else 
    {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        LOG_INFO << "pid = " << getpid() << ", tid = " << ss.str().c_str();
        Logger::SetLogLevel(Logger::LogLevel::kWarn);
        
        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress listen_addr(ip, port);
        int threads_num = atoi(argv[3]);
        
        EventLoop loop;
        
        TcpServer server(&loop, listen_addr, "PingPong");
        
        
        server.SetConnectionCallback(OnConnection);
        server.SetMessageCallback(OnMessage);
        
        if (threads_num > 1)
        {
            server.SetThreadNum(threads_num);
        }
        server.Start();
        loop.Loop();
    }
}