//
// Created by xi on 19-2-7.
//
#include <assert.h>
#include <stdio.h>

#include <blaze/net/EventLoopThread.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/EventLoopThreadPool.h>

namespace blaze
{
namespace net
{

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop, const std::string_view& name) :
    base_loop_(base_loop),
    name_(name),
    started_(false),
    threads_num_(0),
    next_(0)
{}

// NOTE: Do NOT delete element in loops_, they are stack variables in EventLoopThread::ThreadFunc()
EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::Start(const ThreadInitCallback& cb)
{
    assert(!started_);
    base_loop_->AssertInLoopThread();
    started_ = true;
    for (int i = 0; i < threads_num_; ++i)
    {
        char buf[name_.size() + 32];
        snprintf(buf, sizeof(buf), "%s%d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->StartLoop());
    }
    if (threads_num_ == 0 && cb)
    {
        cb(base_loop_);
    }
}

EventLoop* EventLoopThreadPool::GetNextLoop()
{
    base_loop_->AssertInLoopThread();
    assert(started_);
    EventLoop* loop = base_loop_;
    if (!loops_.empty())
    {
        // FIXME: any non-trivial method ?
        // round-robin
        loop = loops_[next_];
        ++next_;
        if (implicit_cast<size_t>(next_) >= loops_.size())
        {
            next_ = 0;
        }
    }
    return loop;
}

EventLoop* EventLoopThreadPool::GetLoopOfHash(size_t hash_value)
{
    base_loop_->AssertInLoopThread();
    EventLoop* loop = base_loop_;
    if (!loops_.empty())
    {
        loop = loops_[hash_value % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::GetAllLoops()
{
    base_loop_->AssertInLoopThread();
    assert(started_);
    if (loops_.empty())
    {
        return std::vector<EventLoop*>(1, base_loop_);
    }
    else
    {
        return loops_;
    }
}

}
}