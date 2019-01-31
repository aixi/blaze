//
// Created by xi on 19-1-31.
//

#include <poll.h>
#include <blaze/net/Channel.h>
#include <blaze/log/Logging.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/PollPoller.h>

namespace blaze
{

namespace net
{

PollPoller::PollPoller(EventLoop* loop) :
    owner_loop_(loop)
{}

PollPoller::~PollPoller()
{}

Timestamp PollPoller::Poll(int timeout, ChannelList* active_channels)
{
    int events_num = ::poll(pollfds_.data(), pollfds_.size(), timeout);
    Timestamp now(Timestamp::Now());
    if (events_num > 0)
    {
        LOG_TRACE << events_num << " events happened";
        FillActiveChannels(events_num, active_channels);
    }
    else if (events_num == 0)
    {
        LOG_TRACE << "nothing happened";
    }
    else
    {
        LOG_SYSERR << "PollPoller::Poll()";
    }
    return now;
}

void PollPoller::FillActiveChannels(int num_events, ChannelList* active_channels)
{
    for (auto pfd = pollfds_.begin(); pfd != pollfds_.end() && num_events > 0; ++pfd)
    {
        if (pfd->revents > 0)
        {
            --num_events;
            auto ch = channels_.find(pfd->fd);
            assert(ch != channels_.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            active_channels->push_back(channel);
        }
    }
}

void PollPoller::UpdateChannel(Channel* channel)
{
    AssertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << "events = " << channel->events();
    if (channel->index() < 0)
    {
        // a new one add to pollfds_
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
        // update a existing one
        assert(channels_.find(channel->fd()) != channels_.end());
        assert(channels_[channel->fd()] == channel);
        int idx = channel->index();
        assert(idx >= 0 && idx < static_cast<int>(pollfds_.size()));
        struct pollfd& pfd = pollfds_[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);
        pfd.fd = channel->fd();
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if (channel->IsNoneEvent())
        {
            // ignore this pollfd
            pfd.fd = -channel->fd() - 1;
        }
    }
}

void PollPoller::AssertInLoopThread()
{
    owner_loop_->AssertInLoopThread();
}

}

}
