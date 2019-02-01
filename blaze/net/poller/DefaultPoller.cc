//
// Created by xi on 19-2-1.
//

#include <stdlib.h>
#include <blaze/net/Poller.h>
#include <blaze/net/poller/PollPoller.h>

using namespace blaze::net;

Poller* Poller::NewDefaultPoller(EventLoop* loop)
{
    return new PollPoller(loop);
}