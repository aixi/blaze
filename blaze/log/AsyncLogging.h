//
// Created by xi on 19-1-22.
//

#ifndef BLAZE_ASYNCLOGGING_H
#define BLAZE_ASYNCLOGGING_H

#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <blaze/concurrent/BlockingQueue.h>
#include <blaze/concurrent/BoundedBlockingQueue.h>
#include <blaze/concurrent/CountDownLatch.h>
#include <blaze/log/LogStream.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

class AsyncLogging : public noncopyable
{
public:

    AsyncLogging(const std::string& basename, off_t roll_size, int flush_interval = 3);

    ~AsyncLogging();

    void Append(const char* logline, int len);

    void Start();

    void Stop();

private:

    void ThreadFunc();

    using Buffer = blaze::detail::FixedBuffer<blaze::detail::kLargeBuffer>;
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;

private:
    const int flush_interval_;
    std::atomic<bool> running_;
    std::string basename_;
    off_t roll_size_;
    blaze::CountDownLatch latch_;
    std::mutex mutex_;
    std::condition_variable cond_;
    BufferPtr current_buffer_;
    BufferPtr next_buffer_;
    BufferVector buffers_;
    std::unique_ptr<std::thread> thread_;
};

}

#endif //BLAZE_ASYNCLOGGING_H
