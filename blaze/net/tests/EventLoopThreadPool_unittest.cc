//
// Created by xi on 19-2-7.
//

#include <stdio.h>
#include <unistd.h>
#include <assert.h>

#include <thread>

#include <blaze/net/EventLoop.h>
#include <blaze/net/EventLoopThreadPool.h>

using namespace blaze;
using namespace blaze::net;

void Print(EventLoop* loop = nullptr)
{
    printf("main(): pid = %d, thread id = %zu, loop = %p\n",
           getpid(),
           std::hash<std::thread::id>()(std::this_thread::get_id()),
           loop);
}

void Init(EventLoop* loop)
{
    printf("Init() pid = %d, thread id = %zu, loop = %p\n",
           getpid(),
           std::hash<std::thread::id>()(std::this_thread::get_id()),
           loop);
}

int main()
{
    Print();
    EventLoop loop;
    loop.RunAfter(11, std::bind(&EventLoop::Quit, &loop));

    {
        printf("Single thread %p:\n", &loop);
        EventLoopThreadPool model(&loop, "single");
        model.SetThreadNum(0);
        model.Start(Init);
        assert(model.GetNextLoop() == &loop);
        assert(model.GetNextLoop() == &loop);
        assert(model.GetNextLoop() == &loop);
    }

    {
        printf("Another thread:\n");
        EventLoopThreadPool model(&loop,"another");
        model.SetThreadNum(1);
        model.Start(Init);
        EventLoop* next_loop = model.GetNextLoop();
        next_loop->RunAfter(2, std::bind(Print, next_loop));
        assert(next_loop != &loop);
        assert(next_loop == model.GetNextLoop());
        assert(next_loop == model.GetNextLoop());
        ::sleep(3);
    }

    {
        printf("Three threads:\n");
        EventLoopThreadPool model(&loop, "three");
        model.SetThreadNum(3);
        model.Start(Init);
        EventLoop* next_loop = model.GetNextLoop();
        next_loop->RunInLoop(std::bind(Print, next_loop));
        assert(next_loop != &loop);
        assert(next_loop != model.GetNextLoop());
        assert(next_loop != model.GetNextLoop());
        assert(next_loop == model.GetNextLoop());
    }
}