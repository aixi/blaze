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
    calling_expired_timers_(false),
    timers_()
{
    // FIXME: where is the Timestamp receive_time parameter ?
    timerfd_channel_.SetReadCallback(std::bind(&TimerQueue::HandleRead, this));
    // Always reading the timerfd, disarm it with timerfd_settime
    timerfd_channel_.EnableReading();
}

TimerQueue::~TimerQueue()
{
    timerfd_channel_.DisableAll();
    // TODO: unregister timerfd_channel_ in poller
    ::close(timerfd_);
    // Do not remove channel, since we're in EventLoop::~EventLoop(). EventLoop owns TimerQueue
    for (const Entry& entry : timers_)
    {
        delete entry.second;
    }
}

TimerId TimerQueue::AddTimer(TimerCallback cb, Timestamp when, double interval)
{
    Timer* timer = new Timer(std::move(cb), when, interval);
    loop_->RunInLoop([this, timer](){AddTimerInLoop(timer);});
    return TimerId(timer, timer->sequence());
}

std::vector<TimerQueue::Entry> TimerQueue::GetExpired(Timestamp when)
{
    std::vector<Entry> expired;
    Entry sentry{when, reinterpret_cast<Timer*>(UINTPTR_MAX)};
    auto it = timers_.lower_bound(sentry);
    assert(it == timers_.end() || when < it->first);
    std::copy(timers_.begin(), it, std::back_inserter(expired));
    return expired; // RVO
}

void TimerQueue::HandleRead()
{
    loop_->AssertInLoopThread();
    Timestamp now(Timestamp::Now());
    detail::ReadTimerfd(timerfd_, now);
    std::vector<Entry> expired = GetExpired(now);
    calling_expired_timers_ = true;
    for (const Entry& entry : expired)
    {
        entry.second->Run();
    }
    calling_expired_timers_ = false;
}

void TimerQueue::AddTimerInLoop(Timer* timer)
{

}

} // namespace net

} // namespace blaze