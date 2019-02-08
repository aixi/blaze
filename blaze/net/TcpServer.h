//
// Created by xi on 19-2-4.
//

#ifndef BLAZE_TCPSERVER_H
#define BLAZE_TCPSERVER_H

#include <string>
#include <memory>
#include <map>
#include <string_view>
#include <blaze/net/Callbacks.h>
#include <blaze/utils/noncopyable.h>

namespace blaze
{
namespace net
{

class EventLoop;
class Acceptor;
class InetAddress;
class EventLoopThreadPool;

class TcpServer : public noncopyable
{
public:
    using ThreadInitCallback = std::function<void (EventLoop*)>;

    TcpServer(EventLoop* loop, const InetAddress& listen_addr, const std::string_view& name);

    ~TcpServer(); // forced out-of-line dtor, for std::unique_ptr member

    /// Set the number of threads for handling input.
    ///
    /// Always accepts new connection in loop's thread.
    /// Must be called before Start()
    /// @param threads_num
    /// - 0 means all I/O in loop's thread, no thread will created.
    ///   this is the default value.
    /// - 1 means all I/O in another thread.
    /// - N means a thread pool with N threads, new connections
    ///   are assigned on a round-robin basis.

    void SetThreadNum(int threads_num);

    // It's harmless to call it multiple times
    // Thread safe
    void Start();

    const std::string& IpPort() const
    {
        return ip_port_;
    }

    const std::string& name() const
    {
        return name_;
    }

    EventLoop* GetLoop() const
    {
        return loop_;
    }

    // valid after calling Start()
//    std::shared_ptr<EventLoopThreadPool> ThreadPool()
//    {
//        return thread_pool_;
//    }

    void SetThreadInitCallback(const ThreadInitCallback& cb)
    {
        thread_init_callback_ = cb;
    }

    // NOT thread safe
    void SetConnectionCallback(const ConnectionCallback& cb)
    {
        connection_callback_ = cb;
    }

    // NOT thread safe
    void SetMessageCallback(const MessageCallback& cb)
    {
        message_callback_ = cb;
    }

    // NOT thread safe
    void SetWriteCompleteCallback(const WriteCompleteCallback& cb)
    {
        write_complete_callback_ = cb;
    }

private:
    // Not thread safe, but in loop
    void NewConnection(int connfd, const InetAddress& peer_addr);
    // Thread safe
    void RemoveConnection(const TcpConnectionPtr& conn);
    // Not thread safe, but in loop thread
    void RemoveConnectionInLoop(const TcpConnectionPtr& conn);

private:
    EventLoop* loop_; // the acceptor loop
    const std::string name_;
    const std::string ip_port_;
    std::unique_ptr<Acceptor> acceptor_; // avoid exposing Acceptor.h in public header
    int next_conn_id_; // always in thread loop
    std::atomic<int> started_;
    std::shared_ptr<EventLoopThreadPool> thread_pool_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
    WriteCompleteCallback write_complete_callback_;
    ThreadInitCallback thread_init_callback_;

    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
    ConnectionMap connections_;
};

} // namespace net
} // namespace blaze

#endif //BLAZE_TCPSERVER_H
