//
// Created by xi on 19-2-1.
//

#include <stdlib.h>
#include <blaze/net/Poller.h>
#include <blaze/net/poller/PollPoller.h>
#include <blaze/net/poller/EPollPoller.h>

using namespace blaze::net;

Poller* Poller::NewDefaultPoller(EventLoop* loop)
{
    if (::getenv("BLAZE_USE_POLL"))
    {
        return new PollPoller(loop);
    }
    else
    {
        return new EPollPoller(loop);
    }
}