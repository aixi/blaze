//
// Created by xi on 19-2-7.
//

#include <blaze/log/Logging.h>
#include <blaze/net/Channel.h>
#include <blaze/net/EventLoop.h>

#include <functional>
#include <map>
#include <thread>

#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>

using namespace blaze;
using namespace blaze::net;

void print(const char* msg)
{
    static std::map<const char*, Timestamp> lasts;
    Timestamp& last = lasts[msg];
    Timestamp now = Timestamp::Now();
    printf("%s tid %zu %s delay %f\n", now.ToString().c_str(), std::hash<std::thread::id>()(std::this_thread::get_id()),
           msg, TimeDifference(now, last));
    last = now;
}

namespace blaze
{
namespace net
{
namespace detail
{
int CreateTimerfd();
void ReadTimerfd(int timerfd, Timestamp now);
}
}
}

// Use relative time, immunized to wall clock changes.
class PeriodicTimer
{
public:
    PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb)
        : loop_(loop),
          timerfd_(blaze::net::detail::CreateTimerfd()),
          timerfdChannel_(loop, timerfd_),
          interval_(interval),
          cb_(cb)
    {
        timerfdChannel_.SetReadCallback(
            std::bind(&PeriodicTimer::handleRead, this));
        timerfdChannel_.EnableReading();
    }

    void start()
    {
        struct itimerspec spec;
        bzero(&spec, sizeof spec);
        spec.it_interval = toTimeSpec(interval_);
        spec.it_value = spec.it_interval;
        int ret = ::timerfd_settime(timerfd_, 0 /* relative timer */, &spec, NULL);
        if (ret)
        {
            LOG_SYSERR << "timerfd_settime()";
        }
    }

    ~PeriodicTimer()
    {
        timerfdChannel_.DisableAll();
        timerfdChannel_.Remove();
        ::close(timerfd_);
    }

private:
    void handleRead()
    {
        loop_->AssertInLoopThread();
        blaze::net::detail::ReadTimerfd(timerfd_, Timestamp::Now());
        if (cb_)
            cb_();
    }

    static struct timespec toTimeSpec(double seconds)
    {
        struct timespec ts;
        bzero(&ts, sizeof ts);
        const int64_t kNanoSecondsPerSecond = 1000000000;
        const int kMinInterval = 100000;
        int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
        if (nanoseconds < kMinInterval)
            nanoseconds = kMinInterval;
        ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
        ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
        return ts;
    }

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    const double interval_; // in seconds
    TimerCallback cb_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << getpid() << ", tid = " << std::hash<std::thread::id>()(std::this_thread::get_id())
             << " Try adjusting the wall clock, see what happens.";
    EventLoop loop;
    PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
    timer.start();
    loop.RunEvery(1, std::bind(print, "EventLoop::runEvery"));
    loop.Loop();
}
