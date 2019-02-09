//
// Created by xi on 19-1-21.
//

#ifndef BLAZE_THREADPOOL_H
#define BLAZE_THREADPOOL_H

#include <assert.h>
#include <deque>
#include <vector>

#include <mutex>
#include <condition_variable>
#include <thread>
#include <future>
#include <functional>

#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Types.h>

namespace blaze
{

class ThreadPool : public noncopyable
{
public:
    using Task = std::function<void()>;

    explicit ThreadPool(const std::string& name = std::string("ThreadPool"));

    ~ThreadPool();

    // must be called before Start()

    void SetMaxTaskSize(size_t max_task_size)
    {
        max_task_size_ = max_task_size;
    }

    void SetThreadInitCallback(const Task& cb)
    {
        thread_init_callback_ = cb;
    }

    void Start(int num_threads);

    void Stop();

    const std::string& Name() const
    {
        return name_;
    }

    size_t TaskSize() const;

    // Could block if max_tasks_size > 0

    template <typename F, typename... Args>
    std::future<std::result_of_t<F(Args...)>> Run(F &&f, Args &&... args);

private:

    bool IsFullUnlock() const;
    void RunInThread();
    Task Take();

private:

    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    std::string name_;
    Task thread_init_callback_;
    std::vector<std::unique_ptr<std::thread>> threads_;
    std::deque<Task> tasks_;
    size_t max_task_size_;
    bool running_;
};

// NOTE: the std::future cannot chain or register callback
// using facebook's folly::future instead
template <typename F, typename... Args>
std::future<std::result_of_t<F(Args...)>> ThreadPool::Run(F&& f, Args&&... args)
{
    using return_type = std::result_of_t<F(Args...)>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();
    if (threads_.empty())
    {
        (*task)();
    }
    else
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (IsFullUnlock())
        {
            not_full_.wait(lock);
        }
        assert(!IsFullUnlock());
        tasks_.emplace_back([task](){(*task)();});
        not_empty_.notify_one();
    }
    return res;
}

}

#endif //BLAZE_THREADPOOL_H
