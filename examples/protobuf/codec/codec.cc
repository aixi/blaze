//
// Created by xi on 19-5-10.
//

#include "codec.h"
#include <blaze/log/Logging.h>
#include <blaze/net/Endian.h>
#include <google/protobuf/descriptor.h>
#include <zlib.h>

using namespace blaze;
using namespace blaze::net;

void ProbufCodec::OnMessage(const TcpConnectionPtr& conn,
                            Buffer* buf,
                            Timestamp receive_time)
{

}

void ProbufCodec::Send(const TcpConnectionPtr& conn, const google::protobuf::Message& message)
{

}

void ProbufCodec::FillEmptyBuffer(Buffer* buf, const google::protobuf::Message& message)
{

}

//MessagePtr ProbufCodec::Parse(const char* buf, int len, ProbufCodec::ErrorCode* error_code)
//{
//    return std::make_shared<google::protobuf::Message>();
//}