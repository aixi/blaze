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

    TimerQueue(EventLoop* loop);

    ~TimerQueue();

    // Schedule the callback to be run at given time,
    // repeat if @c interval > 0.0
    // Must be thread safe. Usually be called from other thread

    TimerId AddTimer(TimerCallback cb, Timestamp when, double interval);

    // void Cancel(TimerId timer_id);

private:

    // FIXME: use std::unique_ptr instead of raw pointer
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;

    // called when timerfd alarms
    void HandleRead();

    // move out all expired timers
    std::vector<Entry> GetExpired(Timestamp when);

    void Reset(const std::vector<Entry>& expired, Timestamp now);

    bool Insert(Timer* timer);

    void AddTimerInLoop(Timer* timer);

private:

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfd_channel_;
    bool calling_expired_timers_;
    // Timer list sorted by expiration
    TimerList timers_;
};

} // namespace net

} // namespace blaze

#endif //BLAZE_TIMERQUEUE_H
