//
// Created by xi on 19-1-17.
//

#ifndef BLAZE_BLOCKINGQUEUE_H
#define BLAZE_BLOCKINGQUEUE_H

#include <assert.h>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Types.h>

namespace blaze
{

template <typename T>
class BlockingQueue : public noncopyable
{
public:
    BlockingQueue() = default;

    void Put(const T& x)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(x);
        not_empty_.notify_one();
    }

    void Put(T&& x)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.emplace_back(std::move(x));
        not_empty_.notify_one();
    }

    T Take()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        // avoid spurious wakeup
        while (queue_.empty())
        {
            not_empty_.wait(lock);
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        return front;
    }

    size_t Size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::deque<T> queue_;
};

}

#endif //BLAZE_BLOCKINGQUEUE_H
