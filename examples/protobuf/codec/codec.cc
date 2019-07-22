//
// Created by xi on 19-5-10.
//

#include <assert.h>
#include <blaze/log/Logging.h>
#include <blaze/net/Endian.h>
#include <zlib.h> // adler32
#include <google/protobuf/descriptor.h>
#include "codec.h"

using namespace blaze;
using namespace blaze::net;

namespace
{

int32_t AsInt32(const char* buf)
{
    int32_t be32 = 0;
    ::memcpy(&be32, buf, sizeof(be32));
    return sockets::NetworkToHost32(be32);
}

}

void ProtobufCodec::DefaultErrorCallback(const TcpConnectionPtr& conn,
                                         Buffer* buf,
                                         Timestamp receive_time,
                                         ProtobufCodec::ErrorCode error)
{
    LOG_ERROR << "ProtobufCodec::DefaultErrorCallback - " << ErrorCodeToString(error);
    if (conn && conn->Connected())
    {
        conn->Shutdown();
    }
}

std::string ProtobufCodec::ErrorCodeToString(ProtobufCodec::ErrorCode code)
{
    switch (code)
    {
        case ProtobufCodec::ErrorCode::kNoError:
            return "NoError";
        case ProtobufCodec::ErrorCode::kInvalidLength:
            return "InvalidLength";
        case ProtobufCodec::ErrorCode::kCheckSumError:
            return "CheckSumError";
        case ProtobufCodec::ErrorCode::kInvalidNameLen:
            return "InvalidNameLength";
        case ProtobufCodec::ErrorCode::kUnknownMessageType:
            return "UnknownMessageType";
        case ProtobufCodec::ErrorCode::kParseError:
            return "ParseError";
        default:
            return "UnknownError";
    }
}

void ProtobufCodec::FillEmptyBuffer(Buffer* buf, const google::protobuf::Message& message)
{
    assert(buf->ReadableBytes() == 0);
    const std::string& type_name = message.GetTypeName();
    int32_t name_len = static_cast<int32_t>(type_name.size() + 1); // end with \0
    buf->AppendInt32(name_len);
    buf->Append(type_name.c_str(), name_len);
    int byte_size = message.ByteSize();
    buf->EnsureWritableSpace(byte_size);
    uint8_t* start = reinterpret_cast<uint8_t*>(buf->BeginWrite());
    uint8_t* end = message.SerializeWithCachedSizesToArray(start);
    UnusedVariable(end);
    assert((end - start) == byte_size);
    buf->HasWritten(byte_size);
    int32_t check_sum = static_cast<int32_t>(
        ::adler32(1, reinterpret_cast<const Bytef*>(buf->Peek()), static_cast<int>(buf->ReadableBytes())));
    buf->AppendInt32(check_sum);
    assert(buf->ReadableBytes() == sizeof(name_len) + name_len + byte_size + sizeof(check_sum));
    int32_t len = sockets::HostToNetwork32(static_cast<int32_t>(buf->ReadableBytes()));
    buf->Prepend(&len, sizeof(len));
}

void ProtobufCodec::OnMessage(const TcpConnectionPtr& conn,
                              Buffer* buf,
                              Timestamp receive_time)
{
    // maybe more than 1 message in buf
    while (buf->ReadableBytes() >= kMinMessageLen + kHeaderLen)
    {
        const int32_t len = buf->PeekInt32();
        if (len > kMaxMessageLen || len < kMinMessageLen)
        {
            error_callback_(conn, buf, receive_time, ProtobufCodec::ErrorCode::kInvalidLength);
            break;
        }
        else if (buf->ReadableBytes() >= implicit_cast<size_t>(len + kHeaderLen))
        {
            ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
            MessagePtr message(Parse(buf->Peek() + kHeaderLen, len, &error_code));
            if (error_code == ProtobufCodec::ErrorCode::kNoError && message)
            {
                message_callback_(conn, message, receive_time);
                buf->Retrieve(kHeaderLen + len);
            }
            else
            {
                error_callback_(conn, buf, receive_time, error_code);
                break;
            }
        }
        else // no enough data to form a complete message
        {
            break;
        }
    }
}

void ProtobufCodec::Send(const TcpConnectionPtr& conn, const google::protobuf::Message& message)
{
    blaze::net::Buffer buf;
    FillEmptyBuffer(&buf, message);
    conn->Send(&buf);
}

// http://www.p-chao.com/2017-09-04/protobuf%E5%8F%8D%E5%B0%84%E8%AF%A6%E8%A7%A3/
// NOTE: Make use of protobuf's reflection ability

google::protobuf::Message* ProtobufCodec::CreateMessage(const std::string& type_name)
{
    google::protobuf::Message* message = nullptr;
    const google::protobuf::Descriptor* descriptor =
        google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
    if (descriptor)
    {
        const google::protobuf::Message* prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        if (prototype)
        {
            message = prototype->New();
        }
    }
    return message;
}

MessagePtr ProtobufCodec::Parse(const char* buf, int len, ProtobufCodec::ErrorCode* error_code)
{
    MessagePtr message;
    int32_t actual_checksum = AsInt32(buf + len - kCheckSumLen);
    int32_t checksum = static_cast<int32_t>(
        ::adler32(1, reinterpret_cast<const Bytef*>(buf), static_cast<int>(len - kCheckSumLen)));
    if (actual_checksum == checksum)
    {
        int32_t name_len = AsInt32(buf);
        // equal 0 means empty protobuf binary message
        if (name_len >= 2 && name_len <= len - kCheckSumLen - kHeaderLen)
        {
            std::string type_name(buf + kNameLen, buf + kHeaderLen + name_len - 1);
            message.reset(CreateMessage(type_name));
            if (message)
            {
                const char* protobuf_binary  = buf + kNameLen + name_len;
                int32_t data_len = len - name_len - kNameLen - kCheckSumLen;
                if (message->ParseFromArray(protobuf_binary, data_len))
                {
                    *error_code = ProtobufCodec::ErrorCode::kNoError;
                }
                else
                {
                    *error_code = ProtobufCodec::ErrorCode::kParseError;
                }
            }
            else
            {
                *error_code = ProtobufCodec::ErrorCode::kUnknownMessageType;
            }
        }
        else
        {
            *error_code = ProtobufCodec::ErrorCode::kInvalidNameLen;
        }
    }
    else
    {
        *error_code = ProtobufCodec::ErrorCode::kCheckSumError;
    }
    return message;
}