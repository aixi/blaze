//
// Created by xi on 19-1-31.
//

#ifndef BLAZE_CHANNEL_H
#define BLAZE_CHANNEL_H

#include <functional>
#include <memory>
#include <blaze/utils/Timestamp.h>
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
    using EventCallback = std::function<void ()>;
    using ReadEventCallback = std::function<void (Timestamp)>;

    Channel(EventLoop* loop, int fd);

    ~Channel();

    void HandleEvent(Timestamp when);

    // Tie this channel object to its owner object managed by shared_ptr
    // prevent the owner object being destroyed in HandleEvent(Timestamp when)
    // std::shared_ptr<void> could hold all types like void*
    void Tie(const std::shared_ptr<void>& obj);

    void SetReadCallback(ReadEventCallback cb)
    {
        read_callback_ = std::move(cb);
    }

    void SetWriteCallback(EventCallback cb)
    {
        write_callback_ = std::move(cb);
    }

    void SetCloseCallback(EventCallback cb)
    {
        close_callback_ = std::move(cb);
    }

    void SetErrorCallback(EventCallback cb)
    {
        error_callback_ = std::move(cb);
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

    void EnableReading()
    {
        events_ |= kReadEvent;
        Update();
    }

    void DisableReading()
    {
        events_ &= ~kReadEvent;
        Update();
    }

    void EnableWriting()
    {
        events_ |= kWriteEvent;
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

    bool IsReading() const
    {
        return events_ & kReadEvent;
    }

    bool IsWriting() const
    {
        return events_ & kWriteEvent;
    }

    bool IsNoneEvent() const
    {
        return events_ == kNoneEvent;
    };

    // for debug
    std::string EventToString() const;
    std::string REventToString() const;

    void Remove();

private:

    void Update();
    void HandleEventWithGuard(Timestamp when);
    std::string EventsToString(int fd, int ev) const;


private:

    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    const int fd_;
    int events_;
    int revents_; // set by ::poll
    int index_; // used by poller

    bool is_in_loop_;
    bool tied_;
    bool event_handling_;

    std::weak_ptr<void> tie_;

    ReadEventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;

};

} // namespace net

} // namespace blaze

#endif //BLAZE_CHANNEL_H
