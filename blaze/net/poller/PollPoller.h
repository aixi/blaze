//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_POLLPOLLER_H
#define BLAZE_POLLPOLLER_H

#include <vector>
#include <blaze/utils/Types.h>
#include <blaze/net/Poller.h>

struct pollfd;

namespace blaze
{

namespace net
{

class Channel;
class EventLoop;

// IO Multiplexing using poll(2)

class PollPoller final : public Poller
{
public:
    using ChannelList = std::vector<Channel*>;

    explicit PollPoller(EventLoop* loop);

    ~PollPoller() override;

    Timestamp Poll(int timeout, ChannelList* active_channels) override;

    void UpdateChannel(Channel* channel) override;

    void RemoveChannel(Channel* channel) override;

    DISABLE_COPY_AND_ASSIGN(PollPoller);

private:

    void FillActiveChannels(int event_nums, ChannelList* active_channels) const;

    // no need to include poll.h
    using PollfdList = std::vector<struct pollfd>;

    PollfdList pollfds_;
};


}

}

#endif //BLAZE_POLLPOLLER_H
