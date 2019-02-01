//
// Created by xi on 19-2-1.
//


#include <blaze/net/Channel.h>
#include <blaze/net/Poller.h>
#include <blaze/net/EventLoop.h>

namespace blaze
{
namespace net
{

Poller::Poller(EventLoop* loop) :
    owner_loop_(loop)
{}

Poller::~Poller() = default;

void Poller::AssertInLoopThread() const
{
    owner_loop_->AssertInLoopThread();
}

bool Poller::HasChannel(Channel* channel) const
{
    AssertInLoopThread();
    auto iter = channels_.find(channel->fd());
    return iter != channels_.end() && iter->second == channel;
}

}
}
