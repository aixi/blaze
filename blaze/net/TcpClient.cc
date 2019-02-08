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

}

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

}
}