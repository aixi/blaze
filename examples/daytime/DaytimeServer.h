//
// Created by xi on 19-4-16.
//

#ifndef BLAZE_DAYTIMESERVER_H
#define BLAZE_DAYTIMESERVER_H

#include <blaze/net/TcpServer.h>

class DaytimeServer
{
public:

    DaytimeServer(blaze::net::EventLoop* loop, const blaze::net::InetAddress& server_addr);

    void Start()
    {
        server_.Start();
    }

private:

    void OnConnection(const blaze::net::TcpConnectionPtr& conn);

    blaze::net::EventLoop* loop_;
    blaze::net::TcpServer server_;
};

#endif //BLAZE_DAYTIMESERVER_H
