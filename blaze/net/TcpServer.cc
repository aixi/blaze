//
// Created by xi on 19-2-4.
//

#include <blaze/log/Logging.h>

#include <blaze/net/SocketsOps.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/EventLoopThreadPool.h>
#include <blaze/net/Acceptor.h>
#include <blaze/net/InetAddress.h>
#include <blaze/net/TcpServer.h>

namespace blaze
{
namespace net
{

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr, const std::string_view& name) :
    loop_(CHECK_NOTNULL(loop)),
    name_(name),
    ip_port_(listen_addr.ToIpPort()),
    acceptor_(new Acceptor(loop_, listen_addr)),
    next_conn_id_(0),
    started_(0),
    thread_pool_(std::make_shared<EventLoopThreadPool>(loop_, name_)),
    connection_callback_(DefaultConnectionCallback),
    message_callback_(DefaultMessageCallback)
{
    acceptor_->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, _1, _2));
}

TcpServer::~TcpServer()
{
    loop_->AssertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";
    for (auto& item : connections_)
    {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->GetLoop()->RunInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
    }
}

void TcpServer::SetThreadNum(int threads_num)
{
    assert(threads_num >= 0);
    thread_pool_->SetThreadNum(threads_num);
}

void TcpServer::Start()
{
    if (started_.exchange(1) == 0)
    {
        assert(!acceptor_->Listening());
        thread_pool_->Start(thread_init_callback_);
        // NOTE: lambda expression can NOT capture class data member acceptor_
        loop_->RunInLoop(std::bind(&Acceptor::Listen, get_pointer(acceptor_)));
    }
}

void TcpServer::NewConnection(int connfd, const InetAddress& peer_addr)
{
    loop_->AssertInLoopThread();
    EventLoop* io_loop = thread_pool_->GetNextLoop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;
    LOG_INFO << "TcpServer::NewConnection [" << name_
             <<"] - new connection [" << conn_name
             <<"] from " << peer_addr.ToIpPort();
    InetAddress local_addr(sockets::GetLocalAddr(connfd));
    // FIXME: poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(io_loop, conn_name, connfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, _1)); // FIXME: unsafe
    io_loop->RunInLoop(std::bind(&TcpConnection::ConnectionEstablished, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn)
{
    // FIXME: unsafe
    loop_->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const blaze::net::TcpConnectionPtr& conn)
{
    loop_->AssertInLoopThread();
    LOG_INFO << "TcpServer::RemoveConnection [" << name_
             << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    assert(n == 1);
    UnusedVariable(n);
    conn->GetLoop()->QueueInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
}

}
}