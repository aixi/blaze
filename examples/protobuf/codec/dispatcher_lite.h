//
// Created by xi on 19-5-11.
//

#ifndef BLAZE_DISPATCHER_LITE_H
#define BLAZE_DISPATCHER_LITE_H

#include <map>
#include <google/protobuf/message.h>
#include <blaze/utils/Types.h>
#include <blaze/net/Callbacks.h>

using MessagePtr = std::shared_ptr<google::protobuf::Message>;
class ProtobufDispatcherLite
{
public:
    using ProtobufMessageCallback = std::function<void (const blaze::net::TcpConnectionPtr&,
                                                        const MessagePtr&,
                                                        blaze::Timestamp)>;
    
    explicit ProtobufDispatcherLite(ProtobufMessageCallback default_cb) :
        default_callback_(std::move(default_cb))
    {
    }
    
    void OnProtobufMessage(const blaze::net::TcpConnectionPtr& conn,
                           const MessagePtr& message, 
                           blaze::Timestamp receive_time) const
    {
        auto iter = callbacks_.find(message->GetDescriptor());
        if (iter != callbacks_.end())
        {
            iter->second(conn, message, receive_time);
        }
        else 
        {
            default_callback_(conn, message, receive_time);
        }
    }
    
    void RegisterMessageCallback(const google::protobuf::Descriptor* descriptor,
                                 const ProtobufMessageCallback& cb)
    {
        callbacks_[descriptor] = cb;
    }
    
    DISABLE_COPY_AND_ASSIGN(ProtobufDispatcherLite);
private:
    using CallbackMap = std::map<const google::protobuf::Descriptor*, ProtobufMessageCallback>;
    CallbackMap callbacks_;
    ProtobufMessageCallback default_callback_;
};

#endif //BLAZE_DISPATCHER_LITE_H
