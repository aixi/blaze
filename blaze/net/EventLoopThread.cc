//
// Created by xi on 19-2-3.
//

#include <assert.h>

#include <blaze/net/EventLoop.h>

#include <blaze/net/EventLoopThread.h>

namespace blaze
{

namespace net
{

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb, const std::string_view& name) :
    loop_(nullptr),
    exiting_(false),
    callback_(cb),
    thread_(nullptr),
    name_(name)
{}

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != nullptr) // not 100% race-free, eg. ThreadFunc could be running callback_
    {
        // still a chance to call nullptr loop_, if ThreadFunc exits just now
        loop_->Quit();
        // thread_->join(); call function detach() or join() of a unjoinable thread leads to UB
        // so we use ThreadGuard to ensure safely destruct of thread object
    }
}

EventLoop* EventLoopThread::StartLoop()
{
    assert(!thread_);
    // if use DtorAction::join, may cause thread blocking
    thread_.reset(new ThreadGuard(ThreadGuard::DtorAction::detach,
                                 std::thread([this]{ThreadFunc();})));
    EventLoop* loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (loop_ == nullptr)
        {
            cond_.wait(lock);
        }
        loop = loop_;
    }
    return loop;
}

void EventLoopThread::ThreadFunc()
{
    EventLoop loop;
    if (callback_)
    {
        callback_(&loop);
    }
    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    loop.Loop();

    std::lock_guard<std::mutex> lock(mutex_);
    loop_ = nullptr;
}

}

}