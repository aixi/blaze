//
// Created by xi on 19-3-15.
//

#ifndef BLAZE_CODEC_H
#define BLAZE_CODEC_H

#include <string_view>
#include <blaze/utils/noncopyable.h>
#include <blaze/log/Logging.h>
#include <blaze/net/Buffer.h>
#include <blaze/net/Endian.h>
#include <blaze/net/TcpConnection.h>

class LengthHeaderCodec : public blaze::noncopyable
{
public:
    using StringMessageCallback = std::function<void (const blaze::net::TcpConnectionPtr&,
                                                      const std::string&, blaze::Timestamp)>;
    explicit LengthHeaderCodec(const StringMessageCallback& cb) :
        message_callback_(cb)
    {}

    void OnMessage(const blaze::net::TcpConnectionPtr& conn,
                   blaze::net::Buffer* buf,
                   blaze::Timestamp receive_time)
    {
        // must retrieve all data in buffer,
        // until the remaining data can not form a message
        while (buf->ReadableBytes() >= kHeaderLength) // kHeaderLen == 4
        {
            const void* data = buf->Peek();
            int32_t be32 = *static_cast<const int32_t*>(data);
            const int32_t len = blaze::net::sockets::NetworkToHost32(be32);
            if (len > 65535 || len < 0)
            {
                LOG_ERROR << "Invalid length " << len;
                conn->Shutdown();
                break;
            }
            else if (buf->ReadableBytes() >= len + kHeaderLength)
            {
                buf->Retrieve(kHeaderLength);
                std::string message(buf->Peek(), len);
                message_callback_(conn, message, receive_time);
                buf->Retrieve(len);
            }
        }
    }

    void Send(const blaze::net::TcpConnectionPtr& conn, const std::string_view& message)
    {
        blaze::net::Buffer buf;
        buf.Append(message.data(), message.size());
        int32_t len = static_cast<int32_t>(message.size());
        int32_t be32 = blaze::net::sockets::HostToNetwork32(len);
        buf.Prepend(&be32, sizeof(be32));
        conn->Send(&buf);
    }

private:
    StringMessageCallback message_callback_;
    static const size_t kHeaderLength = sizeof(int32_t);
};

#endif //BLAZE_CODEC_H
