//
// Created by xi on 19-3-13.
//

#include <stdio.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <blaze/concurrent/CountDownLatch.h>
#include <blaze/concurrent/BlockingQueue.h>

class Test
{
public:
    explicit Test(int num_threads) :
        latch_(num_threads)
    {
        for (int i = 0; i < num_threads; ++i)
        {
            threads_.emplace_back(new std::thread(&Test::ThreadFunc, this));
        }
    }
    
    void Run(int times)
    {
        printf("waiting for count down latch\n");
        latch_.Wait();
        printf("all threads started\n");
        for (int i = 0; i < times; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "hello %d", i);
            std::stringstream ss;
            ss << std::this_thread::get_id();
            printf("tid=%s, put data = %s, size = %zd\n", ss.str().data(), buf, queue_.Size());
        }
    }
    
    void JoinAll()
    {
        for (size_t i = 0; i < threads_.size(); ++i)
        {
            queue_.Put("stop");
        }
        for (auto& thread : threads_)
        {
            thread->join();
        }
    }
    
private:
    void ThreadFunc()
    {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        printf("tid=%s started\n", ss.str().data());
        latch_.CountDown();
        bool running = true;
        while (running)
        {
            std::string d(queue_.Take());
            printf("tid=%s, get data = %s, size = %zd\n", ss.str().data(), d.c_str(), queue_.Size());
            running = (d != "stop");
        }
        printf("tid=%s, stopped\n", ss.str().data());
    }
    
    blaze::BlockingQueue<std::string> queue_;
    blaze::CountDownLatch latch_;
    std::vector<std::unique_ptr<std::thread>> threads_;
};

void TestMove()
{
    blaze::BlockingQueue<std::unique_ptr<int>> queue;
    queue.Put(std::make_unique<int>(42));
    std::unique_ptr<int> x = queue.Take();
    printf("took %d\n", *x);
    *x = 123;
    queue.Put(std::move(x));
    std::unique_ptr<int> y = queue.Take();
    printf("took %d\n", *y);
}

int main()
{
    std::stringstream ss;
    ss << std::this_thread::get_id();
    printf("pid=%d, tid=%s\n", getpid(), ss.str().data());
    Test t(5);
    t.Run(100);
    t.JoinAll();
    TestMove();
    return 0;
}