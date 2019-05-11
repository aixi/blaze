//
// Created by xi on 19-5-11.
//

#include <iostream>
#include "query.pb.h"
#include "dispatcher.h"

using namespace blaze;
using namespace blaze::net;

using QueryPtr = std::shared_ptr<Query>;
using AnswerPtr = std::shared_ptr<Answer>;

void test_down_pointer_cast()
{
    std::shared_ptr<google::protobuf::Message> msg(new Query);
    std::shared_ptr<Query> query(down_pointer_cast<Query>(msg));
    assert(msg && query);
    if (!query)
    {
        abort();
    }
}

void OnQuery(const TcpConnectionPtr& conn, const QueryPtr& message, Timestamp)
{
    std::cout << "OnQuery: " << message->GetTypeName() << std::endl;
}

void OnAnswer(const TcpConnectionPtr& conn, const AnswerPtr& message, Timestamp)
{
    std::cout << "OnAnswer: " << message->GetTypeName() << std::endl;
}

void OnUnknownMessageType(const TcpConnectionPtr& conn, const MessagePtr& message, Timestamp)
{
    std::cout << "OnUnknownMessageType: " << message->GetTypeName() << std::endl;
}

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    test_down_pointer_cast();
    ProtobufDispatcher dispatcher(OnUnknownMessageType);
    dispatcher.RegisterMessageCallback<Query>(OnQuery);
    dispatcher.RegisterMessageCallback<Answer>(OnAnswer);
    TcpConnectionPtr conn;
    Timestamp t;
    std::shared_ptr<Query> query(new Query);
    std::shared_ptr<Answer> answer(new Answer);
    std::shared_ptr<Empty> empty(new Empty);
    dispatcher.OnProtobufMessage(conn, query, t);
    dispatcher.OnProtobufMessage(conn, answer, t);
    dispatcher.OnProtobufMessage(conn, empty, t);
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}