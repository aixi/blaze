//
// Created by xi on 19-5-11.
//

#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <blaze/utils/Types.h>
#include <blaze/log/Logging.h>
#include <blaze/net/TcpClient.h>
#include <blaze/net/EventLoop.h>
#include "codec.h"
#include "dispatcher.h"
#include "query.pb.h"

using namespace blaze;
using namespace blaze::net;

using EmptyPtr = std::shared_ptr<Empty>;
using AnswerPtr = std::shared_ptr<Answer>;

google::protobuf::Message* msg_to_send;

class QueryClient
{
public:

    QueryClient(EventLoop* loop, const InetAddress& server_addr) :
        loop_(loop),
        client_(loop_, server_addr, "QueryClient"),
        dispatcher_(std::bind(&QueryClient::OnUnknownMessage, this, _1, _2, _3)),
        codec_(std::bind(&ProtobufDispatcher::OnProtobufMessage, &dispatcher_, _1, _2, _3))
    {
        dispatcher_.RegisterMessageCallback<Answer>(std::bind(&QueryClient::OnAnswer, this, _1, _2, _3));
        dispatcher_.RegisterMessageCallback<Empty>(std::bind(&QueryClient::OnEmpty, this, _1, _2, _3));
        client_.SetConnectionCallback(std::bind(&QueryClient::OnConnection, this, _1));
        client_.SetMessageCallback(std::bind(&ProtobufCodec::OnMessage, &codec_, _1, _2, _3));
    }

    void Connect()
    {
        client_.Connect();
    }
    DISABLE_COPY_AND_ASSIGN(QueryClient);
private:
    void OnConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->GetLocalAddress().ToIpPort() << " --> "
                 << conn->GetPeerAddress().ToIpPort() << " is "
                 << (conn->Connected() ? "UP" : "DOWN");
        if (conn->Connected())
        {
            codec_.Send(conn, *msg_to_send);
        }
        else
        {
            loop_->Quit();
        }
    }

    void OnUnknownMessage(const TcpConnectionPtr& conn,
                          const MessagePtr& message,
                          Timestamp)
    {
        LOG_INFO << "OnUnknownMessage: " << message->GetTypeName();
    }

    void OnAnswer(const TcpConnectionPtr& conn,
                  const AnswerPtr& message,
                  Timestamp)
    {
        LOG_INFO << "OnAnswer: \n" << message->GetTypeName() << message->DebugString();
    }

    void OnEmpty(const TcpConnectionPtr& conn,
                 const EmptyPtr& message,
                 Timestamp)
    {
        LOG_INFO << "OnEmpty: " << message->GetTypeName();
    }
private:
    EventLoop* loop_;
    TcpClient client_;
    ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << ::getpid();
    if (argc > 2)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        InetAddress server_addr(argv[1], port);
        Query query;
        query.set_id(1);
        query.set_questioner("Xi Ai");
        query.add_question("What's your name ?");
        query.add_question("What time is it now ?");
        Empty empty;
        msg_to_send = &query;
        if (argc > 3 && argv[3][0] == 'e')
        {
            msg_to_send = &empty;
        }
        QueryClient client(&loop, server_addr);
        client.Connect();
        loop.Loop();
    }
    else
    {
        printf("Usage: %s host_ip port [q|e]\n", argv[0]);
    }
}