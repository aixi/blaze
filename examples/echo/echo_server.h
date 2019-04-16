//
// Created by xi on 19-4-16.
//

#ifndef BLAZE_ECHO_SERVER_H
#define BLAZE_ECHO_SERVER_H


#include <blaze/utils/Types.h>
#include <blaze/net/TcpServer.h>
#include <blaze/net/Callbacks.h>
#include <blaze/utils/noncopyable.h>

class EchoServer : public blaze::noncopyable
{
public:

    EchoServer(blaze::net::EventLoop* loop, const blaze::net::InetAddress& server_addr);

    void Start()
    {
        server_.Start();
    }

private:

    void OnConnection(const blaze::net::TcpConnectionPtr& conn);

    void OnMessage(const blaze::net::TcpConnectionPtr& conn,
                      blaze::net::Buffer* buf,
                      blaze::Timestamp time);

    blaze::net::EventLoop* loop_;
    blaze::net::TcpServer server_;
};

#endif //BLAZE_ECHO_SERVER_H
