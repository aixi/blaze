//
// Created by xi on 19-3-13.
//

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <blaze/concurrent/ThreadGuard.h>
#include <blaze/net/EventLoop.h>

using namespace blaze;
using namespace blaze::net;

EventLoop* g_loop;

void Callback()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("callback(): pid = %d, tid = %s\n", getpid(), ss.str().data());
    EventLoop another_loop;
}

void ThreadFunc()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("ThreadFunc(): pid = %d, tid = %s\n", getpid(), ss.str().data());
    assert(EventLoop::GetEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::GetEventLoopOfCurrentThread() == &loop);
    loop.RunAfter(1.0, CallbackBase);
    loop.Loop();
}

int main()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("main(): pid = %d, tid = %s\n", getpid(), ss.str().data());
    assert(EventLoop::GetEventLoopOfCurrentThread() == nullptr);
    EventLoop loop;
    assert(EventLoop::GetEventLoopOfCurrentThread() == &loop);
    ThreadGuard t(ThreadGuard::DtorAction::detach, std::thread(ThreadFunc));
    loop.Loop();
}