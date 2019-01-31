//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_CHANNEL_H
#define BLAZE_CHANNEL_H

#include <functional>
#include <blaze/utils/noncopyable.h>

namespace blaze
{

namespace net
{

// Channel does NOT own fd
// the fd could be socket, eventfd, timerfd, signalfd

class EventLoop;

class Channel : public noncopyable
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);

    void HandleEvent();

    void SetReadCallback(const EventCallback& cb)
    {
        read_callback_ = cb;
    }

    void SetWriteCallback(const EventCallback& cb)
    {
        write_callback_ = cb;
    }

    void SetErrorCallback(const EventCallback& cb)
    {
        error_callback_ = cb;
    }

    int fd() const
    {
        return fd_;
    }

    int events() const
    {
        return events_;
    }

    void set_revents(int revents)
    {
        revents_ = revents;
    }

    int index()
    {
        return index_;
    }

    void set_index(int index)
    {
        index_ = index;
    }

    EventLoop* OwnerLoop()
    {
        return loop_;
    }

    bool IsNoneEvent() const
    {
        return events_ == kNoneEvent;
    };

    void EnableReading()
    {
        events_ |= kReadEvent;
        Update();
    }

    void EnableWriting()
    {
        events_ |= kWriteEvent;
        Update();
    }

    void DisableReading()
    {
        events_ &= ~kReadEvent;
        Update();
    }

    void DisableWriting()
    {
        events_ &= ~kWriteEvent;
        Update();
    }

    void DisableAll()
    {
        events_ = kNoneEvent;
        Update();
    }



private:

    void Update();

private:

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_; // set by ::poll
    int index_; // used by poller

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;

};

} // namespace net

} // namespace blaze

#endif //BLAZE_CHANNEL_H
