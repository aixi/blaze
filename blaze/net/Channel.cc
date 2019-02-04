//
// Created by xi on 19-1-31.
//

#include <poll.h>

#include <blaze/net/EventLoop.h>
#include <blaze/net/Channel.h>
#include <blaze/log/Logging.h>


namespace blaze
{

namespace net
{
const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) :
    loop_(loop),
    fd_(fd),
    events_(0),
    revents_(0),
    index_(-1),
    is_in_loop_(false),
    tied_(false),
    event_handling_(false)
{}

Channel::~Channel()
{
    assert(!event_handling_);
    assert(!is_in_loop_);
    if (loop_->IsInLoopThread())
    {
        assert(!loop_->HasChannel(this));
    }
}

void Channel::HandleEvent()
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            HandleEventWithGuard();
        }
    }
    else
    {
        HandleEventWithGuard();
    }
}

void Channel::Update()
{

    loop_->UpdateChannel(this);
    is_in_loop_ = true;
}

void Channel::Remove()
{
    assert(IsNoneEvent());
    loop_->RemoveChannel(this);
    is_in_loop_ = false;
}

void Channel::HandleEventWithGuard()
{
    event_handling_ = true;
    if (revents_ & POLLNVAL)
    {
        LOG_WARN << "Channel::HandleEvent() POLLNVAL";
    }
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    {
        LOG_WARN << "Channel::HandleEvent() POLLHUP";
        if (close_callback_)
        {
            close_callback_();
        }
    }
    if (revents_ & (POLLERR | POLLNVAL))
    {
        if (error_callback_)
        {
            error_callback_();
        }
    }
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (read_callback_)
        {
            read_callback_();
        }
    }
    if (revents_ & POLLOUT)
    {
        if (write_callback_)
        {
            write_callback_();
        }
    }
    event_handling_ = false;
}

void Channel::Tie(const std::shared_ptr<void>& obj)
{
    tie_ = obj;
    tied_ = true;
}

} // namespace net

} // namespace blaze
