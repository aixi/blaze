//
// Created by xi on 19-1-31.
//
#include <unistd.h>
#include <signal.h>
#include <sys/eventfd.h>

#include <blaze/log/Logging.h>

#include <blaze/net/Poller.h>
#include <blaze/net/Channel.h>
#include <blaze/net/TimerQueue.h>
#include <blaze/net/SocketsOps.h>
#include <blaze/net/EventLoop.h>

namespace
{

// https://www.cnblogs.com/pop-lar/p/5123014.html

thread_local blaze::net::EventLoop* t_loop_of_this_thread = nullptr;

const int kPollTimeMs = 10'000; // 10 seconds

int CreateEventfd()
{
    int fd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (fd < 0)
    {
        LOG_SYSERR << "Failed in ::eventfd()";
        abort();
    }
    return fd;
}


// global/namespace level object will be constructed before main
#pragma GCC diagnostic ignored "-Wold-style-cast"
class IgnoreSigPipe
{
public:
    IgnoreSigPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
        LOG_TRACE << "Ignore SIGPIPE";
    }
};
#pragma GCC diagnostic error "-Wold-style-cast"

// https://www.cnblogs.com/catch/p/4314256.html
// global variable construct before main
IgnoreSigPipe ignoreSigPipe;

} // anonymous namespace

namespace blaze
{
namespace net
{

EventLoop::EventLoop() :
    iteration_(0),
    looping_(false),
    quit_(false),
    calling_pending_tasks_(false),
    event_handling_(false),
    thread_id_(std::this_thread::get_id()),
    poller_(Poller::NewDefaultPoller(this)),
    timer_queue_(new TimerQueue(this)),
    wakeup_fd_(CreateEventfd()),
    wakeup_channel_(new Channel(this, wakeup_fd_)),
    current_active_channel_(nullptr)
{
    LOG_TRACE << "EventLoop " << this << " created " << "in thread " << thread_id_;
    if (t_loop_of_this_thread)
    {
        LOG_FATAL << "Another EventLoop " << t_loop_of_this_thread
                  << " exists in this thread " << thread_id_;
    }
    else
    {
        t_loop_of_this_thread = this;
    }
    // FIXME: where is the Timestamp receive_time parameter ?
    wakeup_channel_->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
    wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "EventLoop " << this << "of thread" << thread_id_
              << " destructs in thread " << std::this_thread::get_id();
    wakeup_channel_->DisableAll();
    wakeup_channel_->Remove();
    ::close(wakeup_fd_);
    t_loop_of_this_thread = nullptr;
}

void EventLoop::Loop()
{
    assert(!looping_);
    AssertInLoopThread();
    looping_ = true;
    quit_ = false; // FIXME: what if someone calls Quit() before Loop() ?
    LOG_TRACE << "EventLoop " << this << " start looping";
    while (!quit_)
    {
        active_channels.clear();
        poll_return_time_ = poller_->Poll(kPollTimeMs, &active_channels);
        ++iteration_;
        event_handling_ = true;
        for (Channel* channel : active_channels)
        {
            current_active_channel_ = channel;
            current_active_channel_->HandleEvent(poll_return_time_);
        }
        current_active_channel_ = nullptr;
        event_handling_ = false;
        DoPendingTasks();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    looping_ = false;
}

void EventLoop::Quit()
{
    quit_ = true;
    if (!IsInLoopThread())
    {
        Wakeup();
    }
}

void EventLoop::Wakeup()
{
    uint64_t one = 1;
    ssize_t n = sockets::write(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::Wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::UpdateChannel(Channel* channel)
{
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    poller_->UpdateChannel(channel);
}

void EventLoop::RemoveChannel(Channel* channel)
{
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    if (event_handling_)
    {
        assert(current_active_channel_ == channel ||
               std::find(active_channels.begin(), active_channels.end(), channel) == active_channels.end());
    }
    poller_->RemoveChannel(channel);
}

bool EventLoop::HasChannel(Channel* channel)
{
    assert(channel->OwnerLoop() == this);
    AssertInLoopThread();
    return poller_->HasChannel(channel);
}

void EventLoop::HandleRead()
{
    uint64_t one = 1;
    ssize_t n = sockets::read(wakeup_fd_, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::HandleRead() reads " << n << " bytes instead of 8";
    }
}

void EventLoop::RunInLoop(Task task)
{
    if (IsInLoopThread())
    {
        task();
    }
    else
    {
        QueueInLoop(std::move(task));
    }
}

void EventLoop::QueueInLoop(Task task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_tasks_.emplace_back(std::move(task));
    }
    if (!IsInLoopThread() || calling_pending_tasks_)
    {
        Wakeup();
    }
}

size_t EventLoop::PendingTasksSize() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return pending_tasks_.size();
}

void EventLoop::DoPendingTasks()
{
    std::vector<Task> tasks;
    calling_pending_tasks_ = true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks.swap(pending_tasks_);
    }
    for (const Task& task : tasks)
    {
        task();
    }
    calling_pending_tasks_ = false;
}

TimerId EventLoop::RunAt(Timestamp when, TimerCallback cb)
{
    return timer_queue_->AddTimer(std::move(cb), when, 0.0);
}

TimerId EventLoop::RunAfter(double delay, TimerCallback cb)
{
    Timestamp when(AddTime(Timestamp::Now(), delay));
    return RunAt(when, std::move(cb));
}

TimerId EventLoop::RunEvery(double interval, TimerCallback cb)
{
    Timestamp when(AddTime(Timestamp::Now(), interval));
    return timer_queue_->AddTimer(std::move(cb), when, interval);
}

void EventLoop::CancelTimer(TimerId timerid)
{
    return timer_queue_->CancelTimer(timerid);
}

void EventLoop::AbortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::AbortNotInLoopThread - EventLoop"
              << "thread_id_ = " << thread_id_
              << " this thread id = " << std::this_thread::get_id();
}

EventLoop* EventLoop::GetEventLoopOfCurrentThread()
{
    return t_loop_of_this_thread;
}

} // namespace blaze
} // namespace net
