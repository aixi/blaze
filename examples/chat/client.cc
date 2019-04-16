//
// Created by xi on 19-3-15.
//

#include "Codec.h"

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <mutex>
#include <blaze/net/EventLoop.h>
#include <blaze/net/EventLoopThread.h>
#include <blaze/net/TcpClient.h>

using namespace blaze;
using namespace blaze::net;

class ChatClient : public noncopyable
{
public:
    ChatClient(EventLoop* loop, const InetAddress& server_addr) :
        client_(loop, server_addr, "ChatClient"),
        codec_(std::bind(&ChatClient::OnStringMessage, this, _1, _2, _3))
    {
        client_.SetConnectionCallback(std::bind(&ChatClient::OnConnection, this, _1));
        client_.SetMessageCallback(std::bind(&LengthHeaderCodec::OnMessage, &codec_, _1, _2, _3));
        client_.EnableRetry();
    }

    void Connect()
    {
        client_.Connect();
    }

    void Disconnect()
    {
        client_.Disconnect();
    }

    void Write(const std::string_view& message)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_)
        {
            codec_.Send(connection_, message);
        }
    }

private:
    void OnConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->GetLocalAddress().ToIpPort() << " -> "
                 << conn->GetPeerAddress().ToIpPort() << " is "
                 << (conn->Connected() ? "UP" : "DOWN");
        std::lock_guard<std::mutex> lock(mutex_);
        if (conn->Connected())
        {
            connection_ = conn;
        }
        else
        {
            connection_.reset();
        }
    }

    void OnStringMessage(const TcpConnectionPtr& conn,
                         const std::string& message,
                         Timestamp)
    {
        printf("<<< %s\n", message.c_str());
    }

private:
    TcpClient client_;
    LengthHeaderCodec codec_;
    std::mutex mutex_;
    TcpConnectionPtr connection_; // GuardedBy mutex_
};



int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid();
    if (argc > 2)
    {
        EventLoopThread loop_thread;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress server_addr(argv[1], port);
        ChatClient client(loop_thread.StartLoop(), server_addr);
        client.Connect();
        std::string line;
        while (std::getline(std::cin, line))
        {
            if (line == std::string("quit"))
            {
                break;
            }
            client.Write(line);
        }
         client.Disconnect();
         std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    else
    {
        printf("Usage: %s server ip port\n", argv[0]);
    }
}