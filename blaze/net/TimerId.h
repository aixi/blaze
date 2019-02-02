//
// Created by xi on 19-2-1.
//

#ifndef BLAZE_TIMERID_H
#define BLAZE_TIMERID_H

#include <blaze/utils/copyable.h>
#include <blaze/net/Timer.h>

namespace blaze
{
namespace net
{

class Timer;

class TimerId : public copyable
{

    friend class TimerQueue;

public:
    TimerId() :
        timer_(nullptr),
        sequence_(0)
    {}

    TimerId(Timer* timer, int64_t seq) :
        timer_(timer),
        sequence_(seq)
    {}

    // default copy-ctor, dtor and copy-assignment are OK

private:
    Timer* timer_;
    int64_t sequence_;
};


}
}

#endif //BLAZE_TIMERID_H
