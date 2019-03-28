//
// Created by xi on 19-2-7.
//

#ifndef BLAZE_CONNECTOR_H
#define BLAZE_CONNECTOR_H

#include <memory>
#include <functional>

#include <blaze/utils/Types.h>

#include <blaze/net/InetAddress.h>

namespace blaze
{
namespace net
{

class Channel;
class EventLoop;

class Connector : public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void (int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& server_addr);

    ~Connector(); // channel_;

    void SetNewConnectionCallback(const NewConnectionCallback& cb)
    {
        new_connection_callback_ = cb;
    }

    void Start(); // can be called in any thread

    void Restart(); // can be called in any thread

    void Stop(); // can be called in any thread

    const InetAddress& ServerAddress() const
    {
        return server_addr_;
    }

private:
    enum class State
    {
        kDisconnected,
        kConnecting,
        kConnected
    };
    static const int kMaxRetryDelayMs;
    static const int kInitRetryDelayMs;

    void SetState(State s)
    {
        state_ = s;
    }

    void StartInLoop();

    void StopInLoop();

    void Connect();

    void Connecting(int sockfd);

    void HandleWrite();

    void HandleError();

    void Retry(int sockfd);

    int RemoveAndResetChannel();

    void ResetChannel();

    DISABLE_COPY_AND_ASSIGN(Connector);

private:
    EventLoop* loop_;
    InetAddress server_addr_;
    bool connect_; // atomic
    State state_;
    int retry_delay_ms_;
    std::unique_ptr<Channel> channel_;
    NewConnectionCallback new_connection_callback_;
};

}
}

#endif //BLAZE_CONNECTOR_H
