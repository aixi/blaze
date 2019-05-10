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
    
}

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    // TODO    
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}