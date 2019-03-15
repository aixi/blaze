#include <sys/timerfd.h>
#include <unistd.h>

#include <blaze/net/Channel.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/EventLoopThread.h>
#include <blaze/log/Logging.h>
#include <blaze/net/SocketsOps.h>
#include <blaze/net/Buffer.h>
#include <blaze/net/InetAddress.h>
#include <blaze/net/Acceptor.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/TcpServer.h>
#include <blaze/net/Callbacks.h>
#include <iostream>
#include <functional>
#include <atomic>

using namespace blaze;
using namespace blaze::net;

EventLoop* g_loop;

//void ThreadFunc()
//{
//    g_loop->Loop();
//}

//void Timeout(int sequence)
//{
//    printf("Timeout\n");
//    //g_loop->Quit();
//}
//
//void Functor()
//{
//    printf("Functor\n");
//    for (int i = 0; i < 10; ++i)
//    {
//        g_loop->RunInLoop([i](){Timeout(i);});
//    }
//    //g_loop->Quit();
//}

//void NewConnection(int sockfd, const InetAddress& peer_addr)
//{
//    printf("NewConnection(): accept a new connection from %s\n", peer_addr.ToIpPort().c_str());
//    ::write(sockfd, "How are you?\n", 13);
//    blaze::net::sockets::close(sockfd);
//}
//
//void NewConnection2(int sockfd, const InetAddress& peer_addr)
//{
//    printf("NewConnection(): accept a new connection from %s\n", peer_addr.ToIpPort().c_str());
//    ::write(sockfd, "How are you2?\n", 13);
//    blaze::net::sockets::close(sockfd);
//}

void OnConnection(const TcpConnectionPtr& conn)
{
    if (conn->Connected())
    {
        printf("OnConnection(): new connection [%s] from %s, at thread id: %zu\n",
               conn->name().c_str(), conn->GetPeerAddress().ToIpPort().c_str(),
               std::hash<std::thread::id>()(std::this_thread::get_id()));
    }
    else
    {
        printf("OnConnection(): connections [%s] is down\n", conn->name().c_str());

    }
}

void OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receive_time)
{
    std::string_view msg = buffer->RetrieveAllAsString();
    printf("OnMessage(): received %zd bytes from connection [%s]: %s at %s\n",
            msg.size(), conn->name().c_str(), msg.data(), receive_time.ToFormattedString().c_str());
}


int main()
{
//    EventLoop loop;
//    g_loop = &loop;
//    std::thread thread(Functor);
//    thread.join();
//    loop.Loop();
//    EventLoop loop;
//    g_loop = &loop;
//    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
//    Channel channel(&loop, timerfd);
//    channel.SetReadCallback(Functor);
//    channel.EnableReading();
//    struct itimerspec howlong;
//    bzero(&howlong, sizeof(howlong));
//    howlong.it_value.tv_sec = 5;
//    howlong.it_interval.tv_sec = 1;
//    ::timerfd_settime(timerfd, 0, &howlong, nullptr);
//    loop.Loop();
//    ::close(timerfd);
//    std::atomic<int64_t> num(0);
//    std::cout << num.fetch_add(1);
//    std::cout << num;
//    auto func = std::bind(Timeout, _1, _2, _3);
//    std::cout << sizeof(func);
//    EventLoopThread* loop_thread = new EventLoopThread;
//    EventLoop* loop = loop_thread->StartLoop();
//    loop->RunInLoop([](){Timeout(0);});
//    delete loop_thread;

//    printf("main(): pid = %d\n", getpid());
//
//    InetAddress listen_addr(9981);
//    InetAddress listen_addr2(9982);
//    EventLoop loop;
//    Acceptor acceptor(&loop, listen_addr);
//    acceptor.SetNewConnectionCallback(NewConnection);
//    acceptor.Listen();
//    Acceptor acceptor2(&loop, listen_addr2);
//    acceptor2.SetNewConnectionCallback(NewConnection2);
//    acceptor2.Listen();
//    loop.Loop();
    printf("main(): pid=%d\n", ::getpid());
    InetAddress listen_addr(9981);
    EventLoop loop;
    TcpServer server(&loop, listen_addr, "test_server");
    server.SetConnectionCallback(OnConnection);
    server.SetMessageCallback(OnMessage);
    //server.SetThreadNum(8);
    server.Start();
    loop.Loop();
    return 0;
}