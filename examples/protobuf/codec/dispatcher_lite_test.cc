//
// Created by xi on 19-5-11.
//

#include <iostream>
#include "query.pb.h"
#include "dispatcher_lite.h"

using namespace blaze;
using namespace blaze::net;

void OnUnknownMessageType(const TcpConnectionPtr& conn,
                          const MessagePtr& message,
                          Timestamp)
{
    std::cout << "OnUnknownMessageType: " << message->GetTypeName() << std::endl;
}

// NOTE: use dispatcher.h instead, no need to down_cast pointer from Message* to Query*

void OnQuery(const TcpConnectionPtr& conn, const MessagePtr& message, Timestamp receive_time)
{
    std::cout << "OnQuery: " << message->GetTypeName() << std::endl;
    std::shared_ptr<Query> query(blaze::down_pointer_cast<Query>(message));
    assert(query != nullptr);
}

void OnAnswer(const TcpConnectionPtr& conn, const MessagePtr& message, Timestamp receive_time)
{
    std::cout << "OnAnswer: " << message->GetTypeName() << std::endl;
    std::shared_ptr<Answer> answer(down_pointer_cast<Answer>(message));
    assert(answer != nullptr);
}

int main()
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    ProtobufDispatcherLite dispatcher_lite(OnUnknownMessageType);
    dispatcher_lite.RegisterMessageCallback(Query::descriptor(), OnQuery);
    dispatcher_lite.RegisterMessageCallback(Answer::descriptor(), OnAnswer);
    std::shared_ptr<Query> query(std::make_shared<Query>());
    std::shared_ptr<Answer> answer(std::make_shared<Answer>());
    std::shared_ptr<Empty> empty(std::make_shared<Empty>());
    TcpConnectionPtr conn;
    Timestamp t;
    dispatcher_lite.OnProtobufMessage(conn, query, t);
    dispatcher_lite.OnProtobufMessage(conn, answer, t);
    dispatcher_lite.OnProtobufMessage(conn, empty, t);
    google::protobuf::ShutdownProtobufLibrary();
    return 0;
}