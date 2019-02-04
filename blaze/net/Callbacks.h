//
// Created by xi on 19-1-14.
//

#ifndef BLAZE_CALLBACKS_H
#define BLAZE_CALLBACKS_H

#include <functional>
#include <blaze/utils/Timestamp.h>

namespace blaze
{

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

namespace net
{

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void ()>;
using ConnectionCallback = std::function<void (const TcpConnectionPtr&)>;
using CloseCallback = std::function<void (const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void (const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void (const TcpConnectionPtr&, size_t)>;

using MessageCallback = std::function<void (const TcpConnectionPtr&, Buffer*, Timestamp)>;

void DefaultConnectionCallback(const TcpConnectionPtr& conn);

void DefaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp receive_time);

} // namespace net
} // namespace blaze

#endif //BLAZE_CALLBACKS_H
