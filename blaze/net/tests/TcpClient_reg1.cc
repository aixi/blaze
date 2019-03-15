//
// Created by xi on 19-3-15.
//

#include <blaze/log/Logging.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/TcpClient.h>

using namespace blaze;
using namespace blaze::net;

TcpClient* g_client;

void Timeout()
{
    LOG_INFO << "timeout";
    g_client->Stop();
}

int main()
{
    EventLoop loop;
    InetAddress server_addr("127.0.0.1", 2); // no such server
    TcpClient client(&loop, server_addr, "TcpClient");
    g_client = &client;
    loop.RunAfter(0.0, Timeout);
    loop.RunAfter(1.0, std::bind(&EventLoop::Quit, &loop));
    client.Connect();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    loop.Loop();
}