//
// Created by xi on 19-1-21.
//

#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <thread>
#include <blaze/log/Logging.h>
#include <blaze/concurrent/ThreadPool.h>
#include <blaze/concurrent/CountDownLatch.h>

void Print()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("thread id = %s\n", ss.str().c_str());
    //LOG_INFO << "thread id = " << std::this_thread::get_id() << '\n';
}

void PrintString(const std::string& str)
{
    LOG_INFO << str;
    usleep(100 * 1000);
}

void test(int max_size)
{
    LOG_WARN << "Test ThreadPool with max queue size = " << max_size;
    blaze::ThreadPool pool("MainThreadPool");
    pool.SetMaxTaskSize(max_size);
    pool.Start(1);
    LOG_WARN << "Add task";
    pool.Submit(Print);
    pool.Submit(Print);
    for (int i = 0; i < 100; ++i)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "task %d", i);
        pool.Submit(PrintString, buf);
    }
    LOG_WARN << "Add task done";
    blaze::CountDownLatch latch(1);
    pool.Submit(&blaze::CountDownLatch::CountDown, std::ref(latch));
    latch.Wait();
    pool.Stop();
}

int main()
{
    //test(0);
    //test(1);
    //test(5);
    test(10);
//    test(50);
    return 0;
}