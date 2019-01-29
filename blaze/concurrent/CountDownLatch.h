//
// Created by xi on 19-1-21.
//

#ifndef BLAZE_COUNTDOWNLATCH_H
#define BLAZE_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>
#include <blaze/utils/noncopyable.h>


namespace blaze
{

class CountDownLatch : public noncopyable
{
public:
    explicit CountDownLatch(int count);

    void Wait();

    void CountDown();

    int GetCount();

private:
    std::mutex mutex_;
    std::condition_variable cond_;
    int count_;
};

}

#endif //BLAZE_COUNTDOWNLATCH_H
