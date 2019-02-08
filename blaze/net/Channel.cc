//
// Created by xi on 19-1-31.
//

#include <sstream>
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

void Channel::HandleEvent(Timestamp when)
{
    std::shared_ptr<void> guard;
    if (tied_)
    {
        guard = tie_.lock();
        if (guard)
        {
            HandleEventWithGuard(when);
        }
    }
    else
    {
        HandleEventWithGuard(when);
    }
}

void Channel::Update()
{
    is_in_loop_ = true;
    loop_->UpdateChannel(this);
}

void Channel::Remove()
{
    assert(IsNoneEvent());
    is_in_loop_ = false;
    loop_->RemoveChannel(this);
}

void Channel::HandleEventWithGuard(Timestamp when)
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
            read_callback_(when);
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

std::string Channel::REventToString() const
{
    return EventsToString(fd_, revents_);
}

std::string Channel::EventToString() const
{
    return EventsToString(fd_, events_);
}

std::string Channel::EventsToString(int fd, int ev) const
{
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN)
        oss << "IN ";
    if (ev & POLLPRI)
        oss << "PRI ";
    if (ev & POLLOUT)
        oss << "OUT ";
    if (ev & POLLHUP)
        oss << "HUP ";
    if (ev & POLLRDHUP)
        oss << "RDHUP ";
    if (ev & POLLERR)
        oss << "ERR ";
    if (ev & POLLNVAL)
        oss << "NVAL ";

    return oss.str();
}

} // namespace net

} // namespace blaze
