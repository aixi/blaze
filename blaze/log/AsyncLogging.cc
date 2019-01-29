//
// Created by xi on 19-1-22.
//

#include <blaze/log/AsyncLogging.h>
#include <blaze/log/LogFile.h>
#include <blaze/utils/Timestamp.h>

using namespace std::chrono_literals;

namespace blaze
{

AsyncLogging::AsyncLogging(const std::string &basename, off_t roll_size, int flush_interval) :
    flush_interval_(flush_interval),
    running_(false),
    basename_(basename),
    roll_size_(roll_size),
    thread_(&AsyncLogging::ThreadFunc, this),
    latch_(1),
    mutex_(),
    cond_(),
    current_buffer_(std::make_unique<Buffer>()),
    next_buffer_(std::make_unique<Buffer>()),
    buffers_()
{
    current_buffer_->bzero();
    next_buffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::Append(const char* logline, int len)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (current_buffer_->Avail() > len)
    {
        current_buffer_->Append(logline, len);
    }
    else
    {
        buffers_.push_back(std::move(current_buffer_));
        if (next_buffer_)
        {
            current_buffer_ = std::move(next_buffer_);
        }
        else
        {
            current_buffer_.reset(new Buffer); // rarely happens
        }
        current_buffer_->Append(logline, len);
        cond_.notify_one();
    }
}

void AsyncLogging::ThreadFunc()
{
    assert(running_);
    latch_.CountDown();
    LogFile output(basename_, roll_size_, false);
    BufferPtr new_buffer1 = std::make_unique<Buffer>();
    BufferPtr new_buffer2 = std::make_unique<Buffer>();
    new_buffer1->bzero();
    new_buffer2->bzero();
    BufferVector buffers_to_write;
    buffers_to_write.reserve(16);
    while (running_)
    {
        assert(new_buffer1 && new_buffer1->Size() == 0);
        assert(new_buffer2 && new_buffer2->Size() == 0);
        assert(buffers_to_write.empty());

        {
            std::cv_status wait_status;
            std::unique_lock<std::mutex> lock(mutex_);
            if (buffers_.empty()) // unusual conditional variable usage
            {
                wait_status = cond_.wait_for(lock, flush_interval_ * 1s);
            }
            buffers_.push_back(std::move(current_buffer_));
            current_buffer_ = std::move(new_buffer1);
            buffers_to_write.swap(buffers_);
            if (!next_buffer_)
            {
                next_buffer_ = std::move(new_buffer2);
            }
        }

        assert(!buffers_to_write.empty());

        if (buffers_to_write.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "Dropped log message at %s, %zd larger buffers",
                Timestamp::Now().ToFormatedString().c_str(), buffers_to_write.size() - 2);
            fputs(buf, stderr);
            output.Append(buf, static_cast<int>(strlen(buf)));
            buffers_to_write.erase(buffers_to_write.begin() + 2, buffers_to_write.end());
        }

        for (auto& buffer : buffers_to_write)
        {
            // FIXME: use unbuffered stdio FILE ? or use ::writev
            output.Append(buffer->Data(), buffer->Size());
        }

        if (buffers_to_write.size() > 2)
        {
            // Drop no-bzero-ed buffers, avoid trashing
            buffers_to_write.resize(2);
        }

        if (!new_buffer1)
        {
            assert(!buffers_to_write.empty());
            new_buffer1 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer1.reset();
        }

        if (!new_buffer2)
        {
            assert(!buffers_to_write.empty());
            new_buffer2 = std::move(buffers_to_write.back());
            buffers_to_write.pop_back();
            new_buffer2.reset();
        }

        buffers_to_write.clear();
        output.Flush();

    }
    output.Flush();
}



}
