//
// Created by xi on 19-1-21.
//

#include <stdio.h>
#include <assert.h>
#include <blaze/utils/Exception.h>
#include <blaze/concurrent/ThreadPool.h>

namespace blaze
{

ThreadPool::ThreadPool(const std::string &name) :
    mutex_(),
    not_empty_(),
    not_full_(),
    name_(name),
    max_task_size_(0),
    running_(false)
{}

ThreadPool::~ThreadPool()
{
    if (running_)
    {
        Stop();
    }
}

void ThreadPool::Start(int num_threads)
{
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(num_threads);
    for (int i = 0; i < num_threads; ++i)
    {
        threads_.emplace_back(new std::thread(&ThreadPool::RunInThread, this));
    }
    if (num_threads == 0 && thread_init_callback_)
    {
        thread_init_callback_();
    }
}

void ThreadPool::Stop()
{
    {
    std::unique_lock<std::mutex> lock(mutex_);
    running_ = false;
    not_empty_.notify_all();
    }
    for (auto& thread : threads_)
    {
        thread->join();
    }
}

size_t ThreadPool::TaskSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return tasks_.size();
}


ThreadPool::Task ThreadPool::Take()
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (tasks_.empty() && running_)
    {
        not_empty_.wait(lock);
    }
    Task task;
    if (!tasks_.empty())
    {
        task = tasks_.front();
        tasks_.pop_front();
        if (max_task_size_ > 0)
        {
            not_full_.notify_one();
        }
    }
    return task;
}

bool ThreadPool::IsFullUnlock() const
{
    return max_task_size_ > 0 && tasks_.size() >= max_task_size_;
}

void ThreadPool::RunInThread()
{
    try
    {
        if (thread_init_callback_)
        {
            thread_init_callback_();
        }
        while (running_)
        {
            Task task(Take());
            if (task)
            {
                task();
            }
        }
    }
    catch (const blaze::Exception& e)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", e.what());
        fprintf(stderr, "stack trace: %s\n", e.StackTrace());
        abort();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "exception caught in ThreadPool %s\n", name_.c_str());
        fprintf(stderr, "reason: %s\n", e.what());
        abort();
    }
    catch (...)
    {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", name_.c_str());
        throw; // rethrow
    }
}

}



















