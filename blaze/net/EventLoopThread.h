//
// Created by xi on 19-2-3.
//

#ifndef BLAZE_EVENTLOOPTHREAD_H
#define BLAZE_EVENTLOOPTHREAD_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <string_view>

#include <blaze/concurrent/ThreadGuard.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

namespace net
{

class EventLoop;

class EventLoopThread : public noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    explicit EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                             const std::string_view& name = std::string());

    ~EventLoopThread();

    EventLoop* StartLoop();

    const std::string name() const
    {
        return name_;
    }


private:

    void ThreadFunc();

private:

    EventLoop* loop_;
    bool exiting_;
    ThreadInitCallback callback_;
    std::unique_ptr<ThreadGuard> thread_;
    std::string name_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

}

}

#endif //BLAZE_EVENTLOOPTHREAD_H
