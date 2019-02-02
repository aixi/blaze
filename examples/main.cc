#include <sys/timerfd.h>
#include <unistd.h>
#include <blaze/net/Channel.h>
#include <blaze/net/EventLoop.h>
#include <blaze/log/Logging.h>
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

void Timeout(int sequence)
{
    printf("Timeout\n");
    //g_loop->Quit();
}

void Functor()
{
    printf("Functor\n");
    for (int i = 0; i < 10; ++i)
    {
        g_loop->RunInLoop([i](){Timeout(i);});
    }
    //g_loop->Quit();
}

int main()
{
    EventLoop loop;
    g_loop = &loop;
    std::thread thread(Functor);
    thread.join();
    loop.Loop();
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

    return 0;
}