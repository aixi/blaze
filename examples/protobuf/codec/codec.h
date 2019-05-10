//
// Created by xi on 19-5-10.
//

#ifndef BLAZE_CODEC_H
#define BLAZE_CODEC_H

#include <string>
#include <string_view>
#include <google/protobuf/message.h>
#include <blaze/utils/Types.h>
#include <blaze/net/Buffer.h>
#include <blaze/net/TcpConnection.h>

//struct ProtobufTransportFormat __attribute__((packed))
//{
//    int32_t len;
//    int32_t name_len;
//    char name[name_len];
//    char proto_binary_data[len - name - 8];
//    int32_t check_sum; // adler32 of name_len, name, proto_binary_data
//}

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

class ProbufCodec
{
public:

    enum class ErrorCode
    {
        kNoError = 0,
        kInvalidLength,
        kCheckSumError,
        kInvalidNameLen,
        kUnknownMessageType,
        kParseError
    };

    using ProtobufMessageCallback = std::function<void (const blaze::net::TcpConnectionPtr&,
                                                        const MessagePtr&,
                                                        blaze::Timestamp)>;

    using ErrorCallback = std::function<void (const blaze::net::TcpConnectionPtr&,
                                              blaze::net::Buffer*,
                                              blaze::Timestamp,
                                              ErrorCode)>;

    explicit ProbufCodec(ProtobufMessageCallback message_callback) :
        message_callback_(std::move(message_callback)),
        error_callback_(DefaultErrorCallback)
    {
    }

    ProbufCodec(ProtobufMessageCallback message_cb, ErrorCallback error_cb) :
        message_callback_(std::move(message_cb)),
        error_callback_(std::move(error_cb))
    {
    }

    void OnMessage(const blaze::net::TcpConnectionPtr& conn,
                   blaze::net::Buffer* buf,
                   blaze::Timestamp receive_time);

    void Send(const blaze::net::TcpConnectionPtr& conn,
              const google::protobuf::Message& message);

    // static const std::string& ErrorCodeToString(ErrorCode code);

    static void FillEmptyBuffer(blaze::net::Buffer* buf, const google::protobuf::Message& message);
    static google::protobuf::Message* CreateMessage(std::string_view type_name);
    static MessagePtr Parse(const char* buf, int len, ErrorCode* error_code);

    DISABLE_COPY_AND_ASSIGN(ProbufCodec);
private:
    static void DefaultErrorCallback(const blaze::net::TcpConnectionPtr& conn,
                                     blaze::net::Buffer* buf,
                                     blaze::Timestamp receive_time,
                                     ErrorCode error);

    ProtobufMessageCallback message_callback_;
    ErrorCallback error_callback_;

    const static int kHeaderLen = sizeof(int32_t);
    const static int KMinMessageLen = 2 * kHeaderLen + 2; // name_len + name + check_sum
    const static int kMaxMessageLen; // FIXME
};

#endif //BLAZE_CODEC_H
