//
// Created by xi on 19-2-7.
//

#ifndef BLAZE_EVENTLOOPTHREADPOLL_H
#define BLAZE_EVENTLOOPTHREADPOLL_H

#include <memory>
#include <vector>
#include <string_view>
#include <functional>

#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Types.h>

namespace blaze
{
namespace net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : public noncopyable
{
public:
    using ThreadInitCallback = std::function<void (EventLoop*)>;

    EventLoopThreadPool(EventLoop* base_loop, const std::string_view& name);

    ~EventLoopThreadPool();

    void SetThreadNum(int threads_num)
    {
        threads_num_ = threads_num;
    }

    void Start(const ThreadInitCallback& cb = ThreadInitCallback());

    // shall be called after Start()
    EventLoop* GetNextLoop();

    // with same hash value, return same EventLoop
    EventLoop* GetLoopOfHash(size_t hash_value);

    std::vector<EventLoop*> GetAllLoops();

    bool started() const
    {
        return started_;
    }

    const std::string& name() const
    {
        return name_;
    }

private:
    EventLoop* base_loop_;
    std::string name_;
    bool started_;
    int threads_num_;
    int next_;

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

}
}

#endif //BLAZE_EVENTLOOPTHREADPOLL_H
