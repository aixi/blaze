//
// Created by xi on 19-2-3.
//

#include <stdio.h>
#include <unistd.h>

#include <chrono>

#include <blaze/log/Logging.h>
#include <blaze/net/EventLoopThread.h>
#include <blaze/net/EventLoop.h>

using namespace blaze;
using namespace blaze::net;

void Print(EventLoop* loop = nullptr)
{
    printf("pid = %d, hash of thread id = %zu, loop = %p\n",
           ::getpid(), std::hash<std::thread::id>()(std::this_thread::get_id()), loop);
}

void PrintThenQuit(EventLoop* loop)
{
    Print(loop);
    loop->Quit();
    std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main()
{
    Print();

    EventLoopThread loop_thread1;


    EventLoopThread loop_thread2;
    EventLoop* loop2 = loop_thread2.StartLoop();
    loop2->RunInLoop([loop2]{Print(loop2);});
    std::this_thread::sleep_for(std::chrono::milliseconds(500));


    EventLoopThread loop_thread3;
    EventLoop* loop3 = loop_thread3.StartLoop();
    loop3->RunInLoop([loop3] { PrintThenQuit(loop3); });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
}