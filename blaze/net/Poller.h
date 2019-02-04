//
// Created by xi on 19-2-1.
//

#ifndef BLAZE_POLLER_H
#define BLAZE_POLLER_H

#include <map>
#include <vector>

#include <blaze/utils/Timestamp.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

namespace net
{

class Channel;
class EventLoop;

// Base class for IO Multiplexing
// This class does not own the Channel object

class Poller : public noncopyable
{
public:

    using ChannelList = std::vector<Channel*>;

    explicit Poller(EventLoop* loop);

    virtual ~Poller();

    // Must be called in loop thread
    // Poll the IO events
    virtual Timestamp Poll(int timeout, ChannelList* active_channels) = 0;

    // Must be called in loop thread
    // Change fd's interested IO events, or add new fd
    virtual void UpdateChannel(Channel* channel) = 0;

    // Must be called in loop thread
    // When Channel object destructs, unregister it from Poller
     virtual void RemoveChannel(Channel* channel) = 0;

    virtual bool HasChannel(Channel* channel) const;

    static Poller* NewDefaultPoller(EventLoop* loop);

    void AssertInLoopThread() const;

protected:

    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* owner_loop_;
};

} // namespace net

} // namespace blaze

#endif //BLAZE_POLLER_H
