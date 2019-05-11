//
// Created by xi on 19-5-11.
//

#include <stdio.h>
#include <unistd.h>
#include <mutex>
#include <blaze/utils/Types.h>
#include <blaze/log/Logging.h>
#include <blaze/net/TcpServer.h>
#include <blaze/net/EventLoop.h>
#include "codec.h"
#include "dispatcher.h"
#include "query.pb.h"

using namespace blaze;
using namespace blaze::net;

using QueryPtr = std::shared_ptr<Query>;
using AnswerPtr = std::shared_ptr<Answer>;

class QueryServer
{
public:
    QueryServer(EventLoop* loop, const InetAddress& server_addr) :
        server_(loop, server_addr, "QueryServer"),
        dispatcher_(std::bind(&QueryServer::OnUnknownMessage, this, _1, _2, _3)),
        codec_(std::bind(&ProtobufDispatcher::OnProtobufMessage, &dispatcher_, _1, _2, _3))
    {
        dispatcher_.RegisterMessageCallback<Query>(std::bind(&QueryServer::OnQuery, this, _1, _2, _3));
        dispatcher_.RegisterMessageCallback<Answer>(std::bind(&QueryServer::OnAnswer, this, _1, _2, _3));
        server_.SetConnectionCallback(std::bind(&QueryServer::OnConnection, this, _1));
        server_.SetMessageCallback(std::bind(&ProtobufCodec::OnMessage, &codec_, _1, _2, _3));
    }

    void Start()
    {
        server_.Start();
    }

    DISABLE_COPY_AND_ASSIGN(QueryServer);

private:
    void OnConnection(const TcpConnectionPtr& conn)
    {
        LOG_INFO << conn->GetLocalAddress().ToIpPort() << " --> "
                 << conn->GetPeerAddress().ToIpPort() << " is "
                 << (conn->Connected() ? "UP" : "DOWN");
    }

    void OnUnknownMessage(const TcpConnectionPtr& conn,
                          const MessagePtr& message,
                          Timestamp)
    {
        LOG_INFO << "OnUnknownMessage: " << message->GetTypeName();
        conn->Shutdown();
    }

    void OnQuery(const TcpConnectionPtr& conn, const QueryPtr& message, Timestamp)
    {
        LOG_INFO << "OnQuery:\n" << message->GetTypeName() << message->DebugString();
        Answer answer;
        answer.set_id(1);
        answer.set_questioner("Xi Ai");
        answer.set_answerer("www.google.com");
        answer.add_solution("Xi Ai");
        answer.add_solution("13:21");
        codec_.Send(conn, answer);
        conn->Shutdown();
    }

    void OnAnswer(const TcpConnectionPtr& conn, const AnswerPtr& message, Timestamp)
    {
        LOG_INFO << "OnAnswer: " << message->GetTypeName();
        conn->Shutdown();
    }

private:
    TcpServer server_;
    ProtobufDispatcher dispatcher_;
    ProtobufCodec codec_;
};

int main(int argc, char* argv[])
{
    LOG_INFO << "pid = " << ::getpid();
    if (argc == 2)
    {
        EventLoop loop;
        uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
        InetAddress server_addr(port);
        QueryServer server(&loop, server_addr);
        server.Start();
        loop.Loop();
    }
    else
    {
        printf("Usage: %s port\n", argv[0]);
    }
}