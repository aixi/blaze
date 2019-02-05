//
// Created by xi on 19-2-4.
//

#include <blaze/log/Logging.h>
#include <blaze/net/SocketsOps.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/Acceptor.h>
#include <blaze/net/InetAddress.h>
#include <blaze/net/TcpServer.h>

namespace blaze
{
namespace net
{

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr, std::string_view name) :
    loop_(loop),
    name_(name),
    ip_port_(listen_addr.ToIpPort()),
    acceptor_(new Acceptor(loop_, listen_addr)),
    started_(false),
    next_conn_id_(0)
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

void TcpServer::Start()
{
    assert(!acceptor_->Listening());
    // NOTE: lambda expression can NOT capture data member acceptor_
    loop_->RunInLoop(std::bind(&Acceptor::Listen, get_pointer(acceptor_)));

}

void TcpServer::NewConnection(int connfd, const InetAddress& peer_addr)
{
    loop_->AssertInLoopThread();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", ip_port_.c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;
    LOG_INFO << "TcpServer::NewConnection [" << name_
             <<"] - new connection [" << conn_name
             <<"] from " << peer_addr.ToIpPort();
    InetAddress local_addr(sockets::GetLocalAddr(connfd));
    // FIXME: poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_, conn_name, connfd, local_addr, peer_addr));
    connections_[conn_name] = conn;
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, _1)); // FIXME: unsafe
    loop_->RunInLoop(std::bind(&TcpConnection::ConnectionEstablished, conn));
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
             << "] - connection " << conn->GetName();
    size_t n = connections_.erase(conn->GetName());
    assert(n == 1);
    UnusedVariable(n);
    loop_->QueueInLoop([conn]{conn->ConnectionDestroyed();});
}

}
}