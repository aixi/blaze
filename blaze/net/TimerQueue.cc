//
// Created by xi on 19-2-1.
//

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif

#include <unistd.h>
#include <sys/timerfd.h>
#include <blaze/log/Logging.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/Timer.h>
#include <blaze/net/TimerId.h>
#include <blaze/net/TimerQueue.h>

namespace blaze
{

namespace net
{

namespace detail
{

int CreateTimerfd()
{
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_SYSERR << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec HowMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.MicrosecondsSinceEpoch() - Timestamp::Now().MicrosecondsSinceEpoch();
    if (microseconds < 100)
    {
        microseconds = 100;
    }
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return ts;
}

void ReadTimerfd(int timerfd, Timestamp when)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));
    if (n != sizeof(howmany))
    {
        LOG_ERROR << "TimerQueue::HandleRead() read " << n << " bytes instead of 8";
    }
}

void ResetTimerfd(int timerfd, Timestamp expiration)
{
    // wakeup EventLoop by timerfd_settime()
    struct itimerspec new_value;
    struct itimerspec old_value;
    bzero(&new_value, sizeof(new_value));
    bzero(&old_value, sizeof(old_value));
    new_value.it_value = HowMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &new_value, &old_value);
    if (ret != 0)
    {
        LOG_SYSERR << "timerfd_settime()";
    }
}

} // namespace detail

TimerQueue::TimerQueue(EventLoop* loop) :
    loop_(loop),
    timerfd_(detail::CreateTimerfd()),
    timerfd_channel_(loop_, timerfd_),
    calling_timer_callbacks_(false)
{
    timerfd_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
    timerfd_channel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    timerfd_channel_.DisableAll();
    timerfd_channel_.Remove();
    ::close(timerfd_);
    // Do not remove channel, since TimerQueue object is a data member of EventLoop, now we are in EventLoop::dtor()
    for (const Entry& entry : timers_)
    {
        delete entry.second;
    }
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->RunInLoop(std::bind(&TimerQueue::AddTimerInLoop, this, timer));
    return TimerId(timer, timer->Sequence());
}

void TimerQueue::CancelTimer(TimerId timerid)
{
    loop_->RunInLoop(std::bind(&TimerQueue::CancelTimerInLoop, this, timerid));
}

void TimerQueue::AddTimerInLoop(Timer* timer)
{
    loop_->AssertInLoopThread();
    bool earliest_timeout = Insert(timer);
    // We insert a timer into timer queue
    // if the new timer timeout earlier than any other old timer
    // we need to reset timer fd timeout
    if (earliest_timeout)
    {
        detail::ResetTimerfd(timerfd_, timer->Expiration());
    }
}

void TimerQueue::CancelTimerInLoop(TimerId timerid)
{
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    ActiveTimer timer(timerid.timer_, timerid.sequence_);
    auto iter = active_timers_.find(timer);
    if (iter != active_timers_.end())
    {
        size_t n = timers_.erase(Entry(iter->first->Expiration(), iter->first));
        assert(n == 1);
        UnusedVariable(n);
        delete iter->first; // FIXME: use std::unique_ptr, no delete
        active_timers_.erase(iter);
    }
    // in case of someone cancel timer in timer callback
    else if (calling_timer_callbacks_)
    {
        canceling_timers_.insert(timer);
    }
    assert(timers_.size() == active_timers_.size());
}

void TimerQueue::HandleRead()
{
    loop_->AssertInLoopThread();
    Timestamp now(Timestamp::Now());
    detail::ReadTimerfd(timerfd_, now);
    std::vector<Entry> expired = GetExpired(now);
    calling_timer_callbacks_ = true;
    canceling_timers_.clear();
    // safe to callback outside critical region
    for (const Entry& entry : expired)
    {
        entry.second->Run();
    }
    calling_timer_callbacks_ = false;
    Reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp when)
{
    assert(timers_.size() == active_timers_.size());
    std::vector<Entry> expired;
    Entry sentry(when, reinterpret_cast<Timer*>(UINTPTR_MAX));
    auto end = timers_.lower_bound(sentry);
    assert(end == timers_.end() || when < end->first);
    std::copy(timers_.begin(), end, std::back_inserter(expired));
    timers_.erase(timers_.begin(), end);
    for (const Entry& entry : expired)
    {
        ActiveTimer timer(entry.second, entry.second->Sequence());
        size_t n = active_timers_.erase(timer);
        assert(n == 1);
        UnusedVariable(n);
    }
    assert(timers_.size() == active_timers_.size());
    return expired; // RVO help us
}

void TimerQueue::Reset(const std::vector<TimerQueue::Entry>& expired, Timestamp now)
{
    Timestamp next_expire;
    for (const Entry& entry : expired)
    {
        ActiveTimer timer(entry.second, entry.second->Sequence());
        if (timer.first->Repeat() && canceling_timers_.find(timer) == canceling_timers_.end())
        {
            timer.first->Restart(now);
            Insert(timer.first);
        }
        else
        {
            // FIXME: move to a free list
            // FIXME: no delete please
            delete timer.first;
        }
    }
    if (!timers_.empty())
    {
        next_expire = timers_.begin()->second->Expiration();
    }
    if (next_expire.IsValid())
    {
        detail::ResetTimerfd(timerfd_, next_expire);
    }
}

bool TimerQueue::Insert(Timer* timer)
{
    loop_->AssertInLoopThread();
    assert(timers_.size() == active_timers_.size());
    bool earliest_timeout = false;
    Timestamp when = timer->Expiration();
    auto begin = timers_.begin();
    if (begin == timers_.end() || when < begin->first)
    {
        earliest_timeout = true;
    }
    std::pair<TimerList::iterator, bool> result1 = timers_.insert(Entry(when, timer));
    assert(result1.second);
    UnusedVariable(result1);
    std::pair<ActiveTimerList::iterator, bool> result2
             = active_timers_.insert(ActiveTimer(timer, timer->Sequence()));
    assert(result2.second);
    UnusedVariable(result2);
    assert(timers_.size() == active_timers_.size());
    return earliest_timeout;
}

} // namespace net

} // namespace blaze