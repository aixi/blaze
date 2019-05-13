//
// Created by xi on 19-5-11.
//

#ifndef BLAZE_DISPATCHER_H
#define BLAZE_DISPATCHER_H

#include <map>
#include <type_traits>
#include <google/protobuf/message.h>
#include <blaze/utils/Types.h>
#include <blaze/net/Callbacks.h>

using MessagePtr = std::shared_ptr<google::protobuf::Message>;

class CallbackBase
{
public:
    CallbackBase() = default;
    virtual ~CallbackBase() = default;

    virtual void OnMessage(const blaze::net::TcpConnectionPtr& conn,
                           const MessagePtr& message,
                           blaze::Timestamp receive_time) const = 0;

    DISABLE_COPY_AND_ASSIGN(CallbackBase);
};

template <typename T>
class CallbackT : public CallbackBase
{
public:
    static_assert(std::is_base_of_v<google::protobuf::Message, T>,
        "T must be derived from google::protobuf::Message.");
    using ProtobufMessageTCallback = std::function<void (const blaze::net::TcpConnectionPtr&,
                                                         const std::shared_ptr<T>& message,
                                                         blaze::Timestamp)>;

    explicit CallbackT(ProtobufMessageTCallback cb) :
        callback_(std::move(cb))
    {
    }

    void OnMessage(const blaze::net::TcpConnectionPtr& conn,
                   const MessagePtr& message,
                   blaze::Timestamp receive_time) const override
    {
        std::shared_ptr<T> concrete(blaze::down_pointer_cast<T>(message));
        assert(concrete != nullptr);
        callback_(conn, concrete, receive_time);
    }

private:
    ProtobufMessageTCallback callback_;
};

class ProtobufDispatcher
{
public:
    using ProtobufMessageCallback = std::function<void (const blaze::net::TcpConnectionPtr&,
                                                         const MessagePtr& message,
                                                         blaze::Timestamp)>;
    explicit ProtobufDispatcher(ProtobufMessageCallback cb) :
        default_callback_(std::move(cb))
    {
    }

    void OnProtobufMessage(const blaze::net::TcpConnectionPtr& conn,
                           const MessagePtr& message,
                           blaze::Timestamp receive_time) const
    {
        auto iter = callbacks_.find(message->GetDescriptor());
        if (iter != callbacks_.end())
        {
            iter->second->OnMessage(conn, message, receive_time);
        }
        else
        {
            default_callback_(conn, message, receive_time);
        }
    }

    template <typename T>
    void RegisterMessageCallback(const typename CallbackT<T>::ProtobufMessageTCallback& cb)
    {
        std::shared_ptr<CallbackT<T>> pd(new CallbackT<T>(cb));
        callbacks_[T::descriptor()] = pd;
    }

    DISABLE_COPY_AND_ASSIGN(ProtobufDispatcher);

private:
    using CallbackMap = std::map<const google::protobuf::Descriptor*, std::shared_ptr<CallbackBase>>;
    CallbackMap callbacks_;
    ProtobufMessageCallback default_callback_;
};

#endif //BLAZE_DISPATCHER_H
