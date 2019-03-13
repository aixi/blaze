//
// Created by xi on 19-3-13.
//
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <blaze/net/EventLoop.h>
#include <blaze/net/EventLoopThread.h>

using namespace blaze;
using namespace blaze::net;

int cnt = 0;
EventLoop* g_loop;

void PrintTid()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("pid = %d, tid = %s\n", getpid(), ss.str().data());
}

void Print(const char* msg)
{
    printf("msg %s %s\n", Timestamp::Now().ToString().c_str(), msg);
    if (++cnt == 20)
    {
        g_loop->Quit();
    }
}

void Cancel(TimerId timer)
{
    g_loop->CancelTimer(timer);
    printf("cancelled timer at %s\n", Timestamp::Now().ToString().c_str());
}

int main()
{
    PrintTid();
    sleep(1);
    {
        EventLoop loop;
        g_loop = &loop;
        Print("main");
        loop.RunAfter(1, [](){Print("once1");});
        loop.RunAfter(1.5, [](){Print("once1.5");});
        loop.RunAfter(2.5, [](){Print("once2.5");});
        loop.RunAfter(3.5, [](){Print("once3.5");});
        TimerId t45 = loop.RunAfter(4.5, [](){Print("once4.5");});
        loop.RunAfter(4.2, [t45](){Cancel(t45);});
        loop.RunAfter(4.8, [t45](){Cancel(t45);});
        loop.RunEvery(2, [](){Print("every2");});
        TimerId t3 = loop.RunEvery(3, [](){Print("every3");});
        loop.RunAfter(9.001, [&t3](){Cancel(t3);});
        loop.Loop();
        printf("main loop exits");
    }
    sleep(1);
    {
        EventLoopThread loop_thread;
        EventLoop* loop = loop_thread.StartLoop();
        loop->RunAfter(2, PrintTid);
        sleep(3);
        printf("thread loop exits");
    }
}