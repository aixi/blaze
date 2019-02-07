//
// Created by xi on 19-2-1.
//

#ifndef BLAZE_TIMERQUEUE_H
#define BLAZE_TIMERQUEUE_H

#include <set>
#include <blaze/net/TimerId.h>
#include <blaze/net/Channel.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

namespace net
{

class EventLoop;
class Timer;

// Best effort, no guarantee that callback will be on time.

class TimerQueue : public noncopyable
{
public:

    explicit TimerQueue(EventLoop* loop);

    ~TimerQueue();

    // Schedule the callback to be run at given time,
    // repeat if @c interval > 0.0
    // Must be thread safe. Usually be called from other thread

    TimerId AddTimer(TimerCallback cb, Timestamp when, double interval);

    void CancelTimer(TimerId timerid);

private:

    // FIXME: use std::unique_ptr instead of raw pointer
    // This requires heterogeneous comparison lookup (N3465) from C++14
    // so that we can find an T* in s std::set<std::unique<T>>
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>; // second is sequence_ to handle same timer object memory address
    using ActiveTimerList = std::set<ActiveTimer>;

    void AddTimerInLoop(Timer* timer);
    void CancelTimerInLoop(TimerId timerid);

    // called when timerfd alarms
    void HandleRead();

    // move out all expired timers
    std::vector<Entry> GetExpired(Timestamp when);
    // Re-insert timer if repeat
    void Reset(const std::vector<Entry>& expired, Timestamp now);

    bool Insert(Timer* timer);

private:

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfd_channel_;
    bool calling_expired_timers_; // for cancel()
    ActiveTimerList active_timers_; // for cancel()
    ActiveTimerList canceling_timers_; // for cancel()
    TimerList timers_; // Timer list sorted by expiration
    // timers_ and active_timers
};

} // namespace net

} // namespace blaze

#endif //BLAZE_TIMERQUEUE_H
