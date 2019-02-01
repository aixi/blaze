#include <sys/timerfd.h>
#include <unistd.h>
#include <blaze/net/Channel.h>
#include <blaze/net/EventLoop.h>
#include <blaze/log/Logging.h>

using namespace blaze;
using namespace blaze::net;

EventLoop* g_loop;

//void ThreadFunc()
//{
//    g_loop->Loop();
//}

void Timeout()
{
    printf("Timeout\n");
    g_loop->Quit();
}

int main()
{
//    EventLoop loop;
//    g_loop = &loop;
//    std::thread thread(ThreadFunc);
//    thread.join();
//    loop.Loop();
    EventLoop loop;
    g_loop = &loop;
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.SetReadCallback(Timeout);
    channel.EnableReading();
    struct itimerspec howlong;
    bzero(&howlong, sizeof(howlong));
    howlong.it_value.tv_sec = 5;
    howlong.it_interval.tv_sec = 1;
    ::timerfd_settime(timerfd, 0, &howlong, nullptr);
    loop.Loop();
    ::close(timerfd);
    return 0;
}