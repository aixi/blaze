//
// Created by xi on 19-1-17.
//

#ifndef BLAZE_BOUNDEDBLOCKINGQUEUE_H
#define BLAZE_BOUNDEDBLOCKINGQUEUE_H

#include <assert.h>
#include <mutex>
#include <condition_variable>
#include <deque>

#include <blaze/utils/noncopyable.h>

namespace blaze
{

template <typename T>
class BoundedBlockingQueue : public noncopyable
{
public:

    explicit BoundedBlockingQueue(size_t max_size) :
        max_size_(max_size)
    {}

    void Put(const T& x)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.size() == max_size_)
        {
            not_full_.wait(lock);
        }
        assert(!queue_.size() < max_size_);
        queue_.push_back(x);
        not_empty_.notify_one();
    }

    void Put(T&& x)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.size() == max_size_)
        {
            not_full_.wait(lock);
        }
        assert(!queue_.size() < max_size_);
        queue_.emplace_back(std::move(x));
        not_empty_.notify_one();
    }

    T Take()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (queue_.empty())
        {
            not_empty_.wait(lock);
        }
        assert(!queue_.empty());
        T front(std::move(queue_.front()));
        queue_.pop_front();
        not_full_.notify_one();
        return front;
    }

    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    bool Full() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size() == max_size_;
    }

    size_t Size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    size_t Capacity() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.capacity();
    }

private:
    const size_t max_size_;
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::deque<T> queue_;
};

}


#endif //BLAZE_BOUNDEDBLOCKINGQUEUE_H
