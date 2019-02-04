//
// Created by xi on 19-2-3.
//

#ifndef BLAZE_ACCEPTOR_H
#define BLAZE_ACCEPTOR_H

#include <functional>

#include <blaze/net/Channel.h>
#include <blaze/net/Socket.h>

#include <blaze/utils/noncopyable.h>

namespace blaze
{
namespace net
{

class EventLoop;
class InetAddress;

class Acceptor : public noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port = true);

    ~Acceptor();

    void SetNewConnectionCallback(const NewConnectionCallback& cb)
    {
        new_connection_callback_ = cb;
    }

    bool Listening() const
    {
        return listening_;
    }

    void Listen();

private:
    void HandleRead();

private:
    EventLoop* loop_;
    Socket listen_socket_;
    Channel listen_channel_;
    bool listening_;
    int idlefd_;
    NewConnectionCallback new_connection_callback_;

};

}
}

#endif //BLAZE_ACCEPTOR_H
