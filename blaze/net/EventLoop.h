//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_EVENTLOOP_H
#define BLAZE_EVENTLOOP_H

#include <thread>
#include <mutex>
#include <vector>
#include <functional>
#include <blaze/utils/Timestamp.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

namespace net
{

class Channel;
class Poller;
class TimerQueue;

class EventLoop : public noncopyable
{
public:

    using Task = std::function<void()>;

    EventLoop();

    ~EventLoop(); // forced out-of-line dtor, for std::unique_ptr member

    void Loop();

    void Quit();

    void Wakeup();

    void UpdateChannel(Channel* channel);

    void RunInLoop(Task task);

    void QueueInLoop(Task task);

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
    void HandleRead(); // wakeup from IO Multiplexing
    void DoPendingTasks();

private:

    int64_t iteration_;
    bool looping_;
    bool quit_;
    bool calling_pending_tasks_;
    bool event_handling_;
    const std::thread::id thread_id_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timer_queue_;
    int wakeupfd_;
    std::unique_ptr<Channel> wakeup_channel_;
    using ChannelList = std::vector<Channel*>;
    ChannelList active_channels;
    mutable std::mutex mutex_;
    std::vector<Task> tasks_; // @GuardedBy mutex_;

    Channel* current_active_channel_;

    Timestamp poll_return_time_;

};

} // namespace net

} // namespace blaze

#endif //BLAZE_EVENTLOOP_H
