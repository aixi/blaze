//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_EVENTLOOP_H
#define BLAZE_EVENTLOOP_H

#include <thread>
#include <vector>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

namespace net
{

class Channel;
class PollPoller;

class EventLoop : public noncopyable
{
public:

    EventLoop();

    ~EventLoop();

    void Loop();

    void UpdateChannel(Channel* channel);

    void Quit();

    void AssertInLoopThread()
    {
        if (!IsInLoopThread())
        {
            AbortNotInLoopThread();
        }
    }

    bool IsInLoopThread() const
    {
        return std::this_thread::get_id() == thread_id_;
    }

    static EventLoop* GetEventLoopOfCurrentThread();

private:

    void AbortNotInLoopThread();

private:
    bool looping_;
    bool quit_;
    const std::thread::id thread_id_;
    std::unique_ptr<PollPoller> poller_;

    using ChannelList = std::vector<Channel*>;
    ChannelList active_channels;

};

} // namespace net

} // namespace blaze

#endif //BLAZE_EVENTLOOP_H
