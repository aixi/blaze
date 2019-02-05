//
// Created by xi on 19-2-5.
//

#ifndef BLAZE_EPOLLPOLLER_H
#define BLAZE_EPOLLPOLLER_H

#include <vector>

#include <blaze/net/Poller.h>

struct epoll_event;

namespace blaze
{
namespace net
{

class EPollPoller : public Poller
{
public:
    explicit EPollPoller(EventLoop* loop);

    ~EPollPoller() override;

    Timestamp Poll(int timeout, ChannelList* active_channels) override;

    void UpdateChannel(Channel* channel) override;

    void RemoveChannel(Channel* channel) override;

private:
    static const int kInitEventListSize;

    static const char* OperationToString(int op);

    void FillActiveChannels(int events_num, ChannelList* active_channels) const;

    void Update(int op, Channel* channel);

private:
    int epollfd_;
    using EventList = std::vector<struct epoll_event>;
    EventList events_;
};

}
}

#endif //BLAZE_EPOLLPOLLER_H
