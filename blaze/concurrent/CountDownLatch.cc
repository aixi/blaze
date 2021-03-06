//
// Created by xi on 19-1-21.
//

#include <blaze/concurrent/CountDownLatch.h>

namespace blaze
{

CountDownLatch::CountDownLatch(int count) :
    count_(count)
{}

void CountDownLatch::Wait()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (count_ > 0)
    {
        cond_.wait(lock);
    }
}

void CountDownLatch::CountDown()
{
    std::lock_guard<std::mutex> lock(mutex_);
    --count_;
    if (count_ == 0)
    {
        // not notify_one()
        cond_.notify_all();
    }
}

int CountDownLatch::GetCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
}

}
