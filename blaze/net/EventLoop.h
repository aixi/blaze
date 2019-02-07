//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_EVENTLOOP_H
#define BLAZE_EVENTLOOP_H

#include <thread>
#include <mutex>
#include <vector>
#include <any>
#include <atomic>
#include <functional>

#include <blaze/utils/Timestamp.h>
#include <blaze/utils/noncopyable.h>
#include <blaze/net/Callbacks.h>
#include <blaze/net/TimerId.h>

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

    Timestamp PollReturnTime() const
    {
        return poll_return_time_;
    }

    int64_t iteration() const
    {
        return iteration_;
    }

    void RunInLoop(Task task);

    void QueueInLoop(Task task);

    void Wakeup();

    size_t PendingTasksSize() const;

    // safe to call from other thread
    TimerId RunAt(Timestamp when, TimerCallback cb);

    // Run callback after @c delay seconds
    // safe to call from other thread
    TimerId RunAfter(double delay, TimerCallback cb);

    // Run callback every @c delay seconds
    // safe to call from other thread
    TimerId RunEvery(double interval, TimerCallback cb);

    // safe to call from other thread
    void CancelTimer(TimerId timerid);

    void UpdateChannel(Channel* channel);

    void RemoveChannel(Channel* channel);

    bool HasChannel(Channel* channel);

    void SetContext(const std::any& context)
    {
        context_ = context;
    }

    const std::any& GetContext() const
    {
        return context_;
    }

    std::any& GetContext()
    {
        return context_;
    }

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

    bool EventHandling() const
    {
        return event_handling_;
    };

    static EventLoop* GetEventLoopOfCurrentThread();

private:

    void AbortNotInLoopThread();
    void HandleRead(); // wakeup from IO Multiplexing
    void DoPendingTasks();

private:

    int64_t iteration_;
    bool looping_;
    std::atomic<bool> quit_;
    bool calling_pending_tasks_;
    bool event_handling_;
    const std::thread::id thread_id_;
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timer_queue_;
    int wakeupfd_;
    std::unique_ptr<Channel> wakeup_channel_;
    Channel* current_active_channel_;
    Timestamp poll_return_time_;
    using ChannelList = std::vector<Channel*>;
    ChannelList active_channels;
    mutable std::mutex mutex_;
    std::vector<Task> pending_tasks_; // @GuardedBy mutex_;
    std::any context_;
};

} // namespace net

} // namespace blaze

#endif //BLAZE_EVENTLOOP_H
