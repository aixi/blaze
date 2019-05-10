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

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    TestQuery();
    puts("");
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}