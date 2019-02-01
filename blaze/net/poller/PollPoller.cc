//
// Created by xi on 19-1-31.
//

#include <poll.h>

#include <blaze/log/Logging.h>

#include <blaze/net/Channel.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/Poller.h>
#include <blaze/net/poller/PollPoller.h>

namespace blaze
{

namespace net
{

PollPoller::PollPoller(EventLoop* loop) :
    Poller(loop)
{}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::Poll(int timeout, ChannelList* active_channels)
{
    // pollfds_ shall not change, change active_channels, active_channels is  EventLoop's data member
    int events_num = ::poll(pollfds_.data(), pollfds_.size(), timeout); // timeout in millisecond
    int saved_errno = errno;
    Timestamp now(Timestamp::Now());
    if (events_num > 0)
    {
        LOG_TRACE << events_num << " events happened";
        FillActiveChannels(events_num, active_channels);
    }
    else if (events_num == 0)
    {
        LOG_TRACE << " nothing happened";
    }
    else
    {
        // slow system call
        // https://www.cnblogs.com/flyfish10000/articles/2576885.html
        if (saved_errno != EINTR)
        {
            errno = saved_errno;
            LOG_SYSERR << "PollPoller::Poll()";
        }
    }
    return now;
}

void PollPoller::UpdateChannel(Channel* channel)
{
    AssertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events" << channel->events();
    if (channel->index() < 0)
    {
        // a new one, add to pollfds_
        assert(channels_.find(channel->fd()) == channels_.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        pollfds_.push_back(pfd);
        int idx = static_cast<int>(pollfds_.size()) - 1;
        channel->set_index(idx);
        channels_[pfd.fd] = channel;
    }
    else
    {
        // update existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->IsNoneEvent())
        {
            // ignore this struct pollfd
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void PollPoller::FillActiveChannels(int event_nums, ChannelList* active_channels) const
{
    for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && event_nums > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --event_nums;
            auto ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            channel->set_revents(pfd->revents);
            active_channels->push_back(channel);
        }
    }
}

}

}
