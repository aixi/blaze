//
// Created by xi on 19-2-7.
//
#include <blaze/log/Logging.h>

#include <blaze/net/Connector.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/SocketsOps.h>
#include <blaze/net/TcpConnection.h>

#include <blaze/net/TcpClient.h>


namespace blaze
{
namespace net
{
namespace detail
{
void RemoveConnection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->QueueInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
}

void RemoveConnector(const ConnectorPtr& connector)
{
    LOG_INFO << "detail::RemoveConnector(const ConnectorPtr& connector)";
}

} // namespace detail

TcpClient::TcpClient(EventLoop* loop,
                     const InetAddress& server_addr,
                     const std::string_view& name) :
    loop_(CHECK_NOTNULL(loop)),
    connector_(std::make_shared<Connector>(loop_, server_addr)),
    name_(name),
    connection_callback_(DefaultConnectionCallback),
    message_callback_(DefaultMessageCallback),
    retry_(false),
    connect_(true),
    next_conn_id_(0)
{
    connector_->SetNewConnectionCallback(
                std::bind(&TcpClient::NewConnection, this, _1));
    // FIXME: SetConnectFailedCallback
    LOG_INFO << "TcpClient::TcpClient[" << name_
             << "] - connector " << get_pointer(connector_);
}

TcpClient::~TcpClient()
{
    LOG_INFO << "TcpClient::~TcpClient[" << name_
             << "] - connector " << get_pointer(connector_);
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn)
    {
        assert(loop_ == conn->GetLoop());
        // FIXME: not 100% safe. if we are in different thread
        CloseCallback cb = std::bind(&detail::RemoveConnection, loop_, _1);
        loop_->RunInLoop(std::bind(&TcpConnection::SetCloseCallback, conn, cb));
        if (unique)
        {
            conn->ForceClose();
        }
    }
    else
    {
        connector_->Stop();
        // FIXME: hack, the RemoveConnector is empty
        loop_->RunAfter(1, std::bind(&detail::RemoveConnector, connector_));
    }
}

void TcpClient::Connect()
{
    // FIXME: check state_
    LOG_INFO << "TcpClient::Connect[" << name_ << "] - connecting to"
             << connector_->ServerAddress().ToIpPort();
    connect_ = true;
    connector_->Start();
}

void TcpClient::Disconnect()
{
    connect_ = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connection_)
        {
            connection_->Shutdown();
        }
    }
}

void TcpClient::Stop()
{
    connect_ = false;
    connector_->Stop();
}

void TcpClient::NewConnection(int sockfd)
{
    loop_->AssertInLoopThread();
    InetAddress peer_addr(sockets::GetPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.ToIpPort().c_str(), next_conn_id_);
    ++next_conn_id_;
    std::string conn_name = name_ + buf;
    InetAddress local_addr(sockets::GetLocalAddr(sockfd));
    // FIXME: poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn(std::make_shared<TcpConnection>(loop_,
                                                          conn_name,
                                                          sockfd,
                                                          local_addr,
                                                          peer_addr));
    conn->SetConnectionCallback(connection_callback_);
    conn->SetMessageCallback(message_callback_);
    conn->SetWriteCompleteCallback(write_complete_callback_);
    conn->SetCloseCallback(std::bind(&TcpClient::RemoveConnection, this, _1)); // FIXME: unsafe
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->ConnectionEstablished();
}

void TcpClient::RemoveConnection(const TcpConnectionPtr& conn)
{
    loop_->AssertInLoopThread();
    assert(loop_ == conn->GetLoop());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }
    loop_->QueueInLoop(std::bind(&TcpConnection::ConnectionDestroyed, conn));
    if (retry_ && connect_)
    {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - reconnecting to "
                 << connector_->ServerAddress().ToIpPort();
        connector_->Restart();
    }
}

} // namespace net
} // namespace blaze