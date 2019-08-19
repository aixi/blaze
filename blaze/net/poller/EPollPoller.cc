//
// Created by xi on 19-2-5.
//
#include <unistd.h>
#include <sys/epoll.h>
#include <poll.h>

#include <blaze/net/Channel.h>
#include <blaze/log/Logging.h>

#include <blaze/net/poller/EPollPoller.h>

// On Linux, the constants of poll(2) and epoll(4)
// are expected to be the same

static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

namespace blaze
{
namespace net
{

const int EPollPoller::kInitEventListSize = 16;

EPollPoller::EPollPoller(EventLoop* loop) :
    Poller(loop),
    epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
    events_(kInitEventListSize)
{
    if (epollfd_ < 0)
    {
        LOG_SYSERR << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::Poll(int timeout, ChannelList* active_channels)
{
    LOG_TRACE << "fd total count " << channels_.size();
    int events_num = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeout);
    Timestamp now(Timestamp::Now());
    int saved_errno = errno;
    if (events_num > 0)
    {
        LOG_TRACE << events_num << " events happened";
        FillActiveChannels(events_num, active_channels);
        // TODO: support epoll watched events number auto shrink ?
        if (implicit_cast<size_t>(events_num) == events_.size())
        {
            events_.resize(events_.size() * 2);
        }
    }
    else if (events_num == 0)
    {
        LOG_TRACE << "nothing happened";
    }
    else
    {
        // error happens, log uncommon ones
        if (saved_errno != EINTR)
        {
            errno = saved_errno;
            LOG_SYSERR << "EPollPoller::Poll()";
        }
    }
    return now;
}

void EPollPoller::FillActiveChannels(int events_num, ChannelList* active_channels) const
{
    assert(implicit_cast<size_t>(events_num) <= events_.size());
    for (int i = 0; i < events_num; ++i)
    {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        auto iter = channels_.find(fd);
        assert(iter != channels_.end());
        assert(iter->second == channel);
#endif
        channel->set_revents(events_[i].events);
        active_channels->push_back(channel);
    }
}

void EPollPoller::UpdateChannel(Channel* channel)
{
    AssertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd()
              << " events = " << channel->events()
              << " index = " << index;
    if (index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        // a new one, EPOLL_CTL_ADD
        if (index == kNew)
        {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        }
        else // index == kDeleted
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);

        }
        channel->set_index(kAdded);
        Update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        UnusedVariable(fd);
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->IsNoneEvent())
        {
            Update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            Update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::RemoveChannel(Channel* channel)
{
    AssertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->IsNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    UnusedVariable(n);
    assert(n == 1);
    if (index == kAdded)
    {
        Update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

// FIXME: always use epoll_ctl(fd, EPOLL_CTL_MODE, ...) to update exists fd
//        ignore returned EEXIST error code

void EPollPoller::Update(int op, Channel* channel)
{
    struct epoll_event event;
    bzero(&event, sizeof(event));
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << OperationToString(op)
        << " fd=" << fd << " event = {" << channel->EventToString() << " }";
    if (::epoll_ctl(epollfd_, op, fd, &event) < 0)
    {
        LOG_SYSERR << "epoll_ctl op = " << OperationToString(op) << " fd = " << fd;
    }
}

const char* EPollPoller::OperationToString(int op)
{
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "EPOLL_CTL_ADD";
        case EPOLL_CTL_MOD:
            return "EPOLL_CTL_MOD";
        case EPOLL_CTL_DEL:
            return "EPOLL_CTL_DEL";
        default:
            assert(false && "ERROR op" && op);
            return "unknown operation";

    }
}

}
}