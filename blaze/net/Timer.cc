//
// Created by xi on 19-2-1.
//

#include <blaze/net/Timer.h>

namespace blaze
{
namespace net
{
std::atomic<int64_t> Timer::s_created_nums_(0);

void Timer::Restart(Timestamp now)
{
    if (repeat_)
    {
        expiration_ = AddTime(now, interval_);
    }
    else
    {
        expiration_ = Timestamp::Invalid();
    }
}

}
}



