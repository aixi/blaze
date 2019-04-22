//
// Created by xi on 19-4-22.
//

#include <blaze/log/Logging.h>
#include <blaze/utils/Types.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/TcpServer.h>
#include <blaze/net/TcpClient.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/utils/Timestamp.h>

using namespace blaze;
using namespace blaze::net;

constexpr size_t frame_len = 2 * sizeof(int64_t);

void ServerOnConnection(const TcpConnectionPtr& conn)
{
    LOG_TRACE << conn->GetName() << ' ' << conn->GetPeerAddress().ToIpPort() << " -> "
              << conn->GetLocalAddress().ToIpPort() << " is "
              << (conn->Connected() ? "UP" : "DOWN");
    if (conn->Connected())
    {
        conn->SetTcpNoDelay(true);
    }
}

void ServerOnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receive_time)
{
    int64_t msg[2];
    while (buf->ReadableBytes() >= frame_len)
    {
        memcpy(msg, buf->Peek(), frame_len);
        buf->Retrieve(frame_len);
        msg[1] = receive_time.MicrosecondsSinceEpoch();
        conn->Send(msg, sizeof(msg));
    }
}