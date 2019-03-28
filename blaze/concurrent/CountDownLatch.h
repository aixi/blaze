//
// Created by xi on 19-1-21.
//

#ifndef BLAZE_COUNTDOWNLATCH_H
#define BLAZE_COUNTDOWNLATCH_H

#include <mutex>
#include <condition_variable>
#include <blaze/utils/Types.h>

namespace blaze
{

class CountDownLatch
{
public:
    explicit CountDownLatch(int count);

    void Wait();

    void CountDown();

    int GetCount() const;

    DISABLE_COPY_AND_ASSIGN(CountDownLatch);

private:
    int count_;
    mutable std::mutex mutex_;
    std::condition_variable cond_;
};

}

#endif //BLAZE_COUNTDOWNLATCH_H
