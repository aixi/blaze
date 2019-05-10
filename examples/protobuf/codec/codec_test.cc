//
// Created by xi on 19-5-10.
//

#include <stdio.h>
#include <zlib.h>
#include "codec.h"
#include "query.pb.h"
#include <blaze/net/Endian.h>

using namespace blaze;
using namespace blaze::net;

void Print(const Buffer& buf)
{
    printf("encoded to %zd bytes\n", buf.ReadableBytes());
    const char* data = buf.Peek();
    for (size_t i = 0; i < buf.ReadableBytes(); ++i)
    {
        unsigned char ch = static_cast<unsigned char>(data[i]);
        printf("%2zd: 0x%02x %c\n", i, ch, isgraph(ch) ? ch : ' ');
    }
}

void TestQuery()
{
    blaze::Query query;
    query.set_id(1);
    query.set_questioner("Xi Ai");
    query.add_question("What's your name");
    query.add_question("What time is it now");
    Buffer buf;
    ProtobufCodec::FillEmptyBuffer(&buf, query);
    Print(buf);

    const int32_t len = buf.ReadInt32();
    assert(len == static_cast<int32_t>(buf.ReadableBytes()));
    ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
    MessagePtr message(ProtobufCodec::Parse(buf.Peek(), len, &error_code));
    assert(error_code == ProtobufCodec::ErrorCode::kNoError);
    assert(message != nullptr);
    assert(message->DebugString() == query.DebugString());
    std::shared_ptr<blaze::Query> new_query(down_pointer_cast<blaze::Query>(message));
    assert(new_query != nullptr);
}

void TestAnswer()
{
    blaze::Answer answer;
    answer.set_id(1);
    answer.set_questioner("Xi Ai");
    answer.set_answerer("www.google.com");
    answer.add_solution("Xi Ai");
    answer.add_solution("20:57");
    Buffer buf;
    ProtobufCodec::FillEmptyBuffer(&buf, answer);
    Print(buf);
    const int32_t len = buf.ReadInt32();
    assert(len == static_cast<int32_t>(buf.ReadableBytes()));
    ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
    MessagePtr message(ProtobufCodec::Parse(buf.Peek(), len, &error_code));
    assert(error_code == ProtobufCodec::ErrorCode::kNoError);
    assert(message != nullptr);
    message->PrintDebugString();
    assert(message->DebugString() == answer.DebugString());
    std::shared_ptr<blaze::Answer> new_answer(down_pointer_cast<blaze::Answer>(message));
    assert(new_answer != nullptr);
}

void TestEmpty()
{
    blaze::Empty empty;
    Buffer buf;
    ProtobufCodec::FillEmptyBuffer(&buf, empty);
    Print(buf);
    const int32_t len = buf.ReadInt32();
    assert(len == static_cast<int32_t>(buf.ReadableBytes()));
    ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
    MessagePtr message(ProtobufCodec::Parse(buf.Peek(), len, &error_code));
    assert(error_code == ProtobufCodec::ErrorCode::kNoError);
    assert(message != nullptr);
    message->PrintDebugString();
    assert(message->DebugString() == empty.DebugString());
}

void RedoCheckSum(std::string& data, int len)
{
    int32_t check_sum = sockets::HostToNetwork32(static_cast<int32_t>(
        ::adler32(1, reinterpret_cast<const Bytef*>(data.c_str()), static_cast<int>(len - 4)))); // FIXME: len - 4
    const char* check_sum_ptr = reinterpret_cast<const char*>(&check_sum);
    data[len - 4] = check_sum_ptr[0];
    data[len - 3] = check_sum_ptr[1];
    data[len - 2] = check_sum_ptr[2];
    data[len - 1] = check_sum_ptr[3];
}

void TestBadBuffer()
{
    blaze::Empty empty;
    empty.set_id(43);
    Buffer buf;
    ProtobufCodec::FillEmptyBuffer(&buf, empty);
    Print(buf);
    const int32_t len = buf.ReadInt32();
    assert(len == static_cast<int32_t>(buf.ReadableBytes()));

    {
        std::string data(buf.Peek(), len);
        ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
        MessagePtr message(ProtobufCodec::Parse(data.c_str(), len - 1, &error_code));
        assert(message == nullptr);
        assert(error_code == ProtobufCodec::ErrorCode::kCheckSumError);
    }

    {
        std::string data(buf.Peek(), len);
        ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
        ++data[len - 1];
        MessagePtr message(ProtobufCodec::Parse(data.c_str(), len, &error_code));
        assert(message == nullptr);
        assert(error_code == ProtobufCodec::ErrorCode::kCheckSumError);
    }

    {
        std::string data(buf.Peek(), len);
        ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
        ++data[0];
        MessagePtr message(ProtobufCodec::Parse(data.c_str(), len, &error_code));
        assert(message == nullptr);
        assert(error_code == ProtobufCodec::ErrorCode::kCheckSumError);
    }

    {
        std::string data(buf.Peek(), len);
        ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
        data[3] = 100;
        RedoCheckSum(data, len);
        MessagePtr message(ProtobufCodec::Parse(data.c_str(), len, &error_code));
        assert(message == nullptr);
        assert(error_code == ProtobufCodec::ErrorCode::kInvalidNameLen);
    }

    {
        std::string data(buf.Peek(), len);
        ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
        --data[3];
        RedoCheckSum(data, len);
        MessagePtr message(ProtobufCodec::Parse(data.c_str(), len, &error_code));
        assert(message == nullptr);
        assert(error_code == ProtobufCodec::ErrorCode::kUnknownMessageType);
    }

    {
        std::string data(buf.Peek(), len);
        ProtobufCodec::ErrorCode error_code = ProtobufCodec::ErrorCode::kNoError;
        data[4] = 'M';
        RedoCheckSum(data, len);
        MessagePtr message(ProtobufCodec::Parse(data.c_str(), len, &error_code));
        assert(message == nullptr);
        assert(error_code == ProtobufCodec::ErrorCode::kUnknownMessageType);
    }

    // TODO: test ParseError
}

int g_count = 0;

void OnMessage(const blaze::net::TcpConnectionPtr& conn,
               const MessagePtr& message,
               blaze::Timestamp receive_time)
{
    ++g_count;
}

void TestOnMessage()
{
    blaze::Query query;
    query.set_id(1);
    query.set_questioner("Xi Ai");
    query.add_question("What's your name ?");

    Buffer buf1;
    ProtobufCodec::FillEmptyBuffer(&buf1, query);

    blaze::Empty empty;
    empty.set_id(43);
    empty.set_id(1982);

    Buffer buf2;
    ProtobufCodec::FillEmptyBuffer(&buf2, empty);

    size_t total_len = buf1.ReadableBytes() + buf2.ReadableBytes();
    Buffer all;
    all.Append(buf1.Peek(), buf1.ReadableBytes());
    all.Append(buf2.Peek(), buf2.ReadableBytes());
    assert(all.ReadableBytes() == total_len);
    blaze::net::TcpConnectionPtr conn;
    blaze::Timestamp t;
    ProtobufCodec codec(OnMessage);
    for (size_t len = 0; len <= total_len; ++len)
    {
        Buffer input;
        input.Append(all.Peek(), len);
        g_count = 0;
        codec.OnMessage(conn, &input, t);
        int expected = len < buf1.ReadableBytes() ? 0 : 1;
        if (len == total_len)
        {
            expected = 2;
        }
        assert(g_count == expected);
        UnusedVariable(expected);
        printf("%2zd %d\n", len, g_count);
        input.Append(all.Peek() + len, total_len - len);
        codec.OnMessage(conn, &input, t);
        assert(g_count == 2);
    }
}

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    TestQuery();
    puts("");
    TestAnswer();
    puts("");
    TestEmpty();
    puts("");
    TestBadBuffer();
    TestOnMessage();
    puts("");
    puts("All pass!");
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}