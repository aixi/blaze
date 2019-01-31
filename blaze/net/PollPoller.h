//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_POLLER_H
#define BLAZE_POLLER_H

#include <vector>
#include <map>
#include <blaze/utils/Timestamp.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

namespace net
{

class Channel;
class EventLoop;

// This class does not own Channel object

class PollPoller : public noncopyable
{
public:

    using ChannelList = std::vector<Channel*>;

    PollPoller(EventLoop* loop);

     ~PollPoller();

    // Must be called in the loop thread
    // timeout in millisecond
    Timestamp Poll(int timeout, ChannelList* active_channels);

    // Change the interested IO events

    void UpdateChannel(Channel* channel);

    //void RemoveChannel(Channel* channel);

    void AssertInLoopThread();

private:

    void FillActiveChannels(int num_events, ChannelList* active_channels);

private:

    using PollfdList = std::vector<struct pollfd>;
    using ChannelMap = std::map<int, Channel*>; // fd to Channel*

    EventLoop* owner_loop_;
    PollfdList pollfds_;
    ChannelMap channels_;
};

}

}

#endif //BLAZE_POLLER_H
