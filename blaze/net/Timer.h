//
// Created by xi on 19-2-1.
//

#ifndef BLAZE_TIMER_H
#define BLAZE_TIMER_H

#include <atomic>

#include <blaze/net/Callbacks.h>
#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Timestamp.h>

namespace blaze
{

namespace net
{


class Timer : public noncopyable
{
public:
    Timer(TimerCallback cb, Timestamp when, double interval) :
        timer_callback_(std::move(cb)),
        expiration_(when),
        interval_(interval),
        repeat_(interval_ > 0.0),
        sequence_(s_created_nums_.fetch_add(1, std::memory_order_relaxed))
    {}

    void Run() const
    {
        timer_callback_();
    }

    void Restart(Timestamp now);

    Timestamp expiration() const
    {
        return expiration_;
    }

    bool repeat() const
    {
        return repeat_;
    }

    int64_t sequence() const
    {
        return sequence_;
    }

    static int64_t CreatedNumber()
    {
        return s_created_nums_.load(std::memory_order_relaxed);
    }

private:

    const TimerCallback timer_callback_;
    Timestamp expiration_;
    const double interval_;
    const bool repeat_;
    const int64_t sequence_;

    static std::atomic<int64_t> s_created_nums_;
};

} // namespace net

} // namespace blaze

#endif //BLAZE_TIMER_H
