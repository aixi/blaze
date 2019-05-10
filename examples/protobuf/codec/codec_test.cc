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

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    TestQuery();
    puts("");
    TestAnswer();
    puts("");
    TestEmpty();
    puts("");
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}