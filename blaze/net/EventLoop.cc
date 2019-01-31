//
// Created by xi on 19-1-31.
//

#include <blaze/log/Logging.h>
#include <blaze/net/PollPoller.h>
#include <blaze/net/Channel.h>
#include <blaze/net/EventLoop.h>

namespace
{

thread_local blaze::net::EventLoop* t_loop_in_this_thread = nullptr;

const int kPollTimeMs = 10'000;

} // anonymous namespace

namespace blaze
{
namespace net
{

EventLoop::EventLoop() :
    looping_(false),
    quit_(false),
    thread_id_(std::this_thread::get_id()),
    poller_(new PollPoller(this))
{
    LOG_TRACE << "EventLoop created " << this << " in thread " << thread_id_;
    if (t_loop_in_this_thread)
    {
        LOG_FATAL << "Another EventLoop " << t_loop_in_this_thread
                  << " exists in this thread " << thread_id_;
    }
    else
    {
        t_loop_in_this_thread = this;
    }
}

EventLoop::~EventLoop()
{
    assert(!looping_);
    t_loop_in_this_thread = nullptr;
}

void EventLoop::UpdateChannel(Channel* channel)
{
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->UpdateChannel(channel);
}

void EventLoop::Loop()
{
    assert(!looping_);
    AssertInLoopThread();
    looping_ = true;
    quit_ = false;

    while (!quit_)
    {
        active_channels.clear();
        poller_->Poll(kPollTimeMs, &active_channels);
        for (Channel* channel : active_channels)
        {
            channel->HandleEvent();
        }
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::Quit()
{
    quit_ = true;
    // Wakeup();
}

void EventLoop::AbortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop"
              << "thread_id_ = " << thread_id_
              << " this thread id = " << std::this_thread::get_id();
}

EventLoop* EventLoop::GetEventLoopOfCurrentThread()
{
    return t_loop_in_this_thread;
}

} // namespace blaze
} // namespace net
