//
// Created by xi on 19-3-13.
//
#include <stdio.h>
#include <unistd.h>
#include <utility>
#include <atomic>
#include <sstream>
#include <blaze/log/Logging.h>
#include <blaze/net/EventLoop.h>
#include <blaze/net/EventLoopThreadPool.h>
#include <blaze/net/InetAddress.h>
#include <blaze/net/Buffer.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/TcpClient.h>

using namespace blaze;
using namespace blaze::net;

class Client;

class Session : public noncopyable
{
public:
    Session(EventLoop* loop, const InetAddress& server_addr,
                             const std::string_view& name,
                             Client* owner) :
        client_(loop, server_addr, name),
        owner_(owner),
        bytes_read_(0),
        bytes_written_(0),
        messages_read_(0)
    {
        client_.SetConnectionCallback(std::bind(&Session::OnConnection, this, _1));
        client_.SetMessageCallback(std::bind(&Session::OnMessage, this, _1, _2, _3));
    }

    void Start()
    {
        client_.Connect();
    }

    void Stop()
    {
        client_.Disconnect();
    }

    int64_t BytesRead() const
    {
        return bytes_read_;
    }

    int64_t MessagesRead() const
    {
        return messages_read_;
    }

private:
    void OnConnection(const TcpConnectionPtr& conn);

    void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp)
    {
        ++messages_read_;
        bytes_read_ += buf->ReadableBytes();
        bytes_written_ += buf->ReadableBytes();
        conn->Send(buf);
    }

    TcpClient client_;
    Client* owner_;
    int64_t bytes_read_;
    int64_t bytes_written_;
    int64_t messages_read_;

};

class Client : public noncopyable
{
public:
    Client(EventLoop* loop, const InetAddress& server_addr,
                            int block_size,
                            int session_count,
                            int timeout,
                            int thread_count) :
        loop_(loop),
        thread_pool_(loop_, "pingpong-client"),
        session_count_(session_count),
        timeout_(timeout),
        num_connected_(0)
    {
        loop_->RunAfter(timeout_, [this]{ HandleTimeout();});
        if (thread_count > 1)
        {
            thread_pool_.SetThreadNum(thread_count);
        }
        thread_pool_.Start();
        for (int i = 0; i < block_size; ++i)
        {
            message_.push_back(static_cast<char>(i % 128));
        }
        for (int i = 0; i < session_count_; ++i)
        {
            char buf[32];
            snprintf(buf, sizeof(buf), "C%05d", i);
            Session* session = new Session(thread_pool_.GetNextLoop(), server_addr, buf, this);
            session->Start();
            sessions_.emplace_back(session);
        }
    }

    const std::string message() const
    {
        return message_;
    }

    void OnConnect()
    {
        if (num_connected_.fetch_add(1) == session_count_)
        {
            LOG_WARN << "all connected";
        }
    }

    void OnDisconnected(const TcpConnectionPtr& conn)
    {
        if (num_connected_.fetch_add(-1) == 0)
        {
            LOG_WARN << "all disconnected";
            int64_t total_bytes_read = 0;
            int64_t total_message_read = 0;
            for (const auto& session : sessions_)
            {
                total_bytes_read += session->BytesRead();
                total_message_read += session->MessagesRead();
            }
            LOG_WARN << total_bytes_read << "total bytes read";
            LOG_WARN << total_message_read << "total messages read";
            LOG_WARN << static_cast<double>(total_bytes_read) / static_cast<double>(total_message_read)
                     << " average message size";
            LOG_WARN << static_cast<double>(total_bytes_read) / (timeout_ * 1024 * 1024)
                     << "MiB/s throughput";
            conn->GetLoop()->QueueInLoop([this]{Quit();});
        }
    }

private:

    void Quit()
    {
        loop_->QueueInLoop(std::bind(&EventLoop::Quit, loop_));
    }

    void HandleTimeout()
    {
        LOG_WARN << "stop";
        for (auto& session : sessions_)
        {
            session->Stop();
        }
    }

    EventLoop* loop_;
    EventLoopThreadPool thread_pool_;
    int session_count_;
    int timeout_;
    std::atomic<int32_t> num_connected_;
    std::vector<std::unique_ptr<Session>> sessions_;
    std::string message_;
};

void Session::OnConnection(const TcpConnectionPtr& conn)
{
    if (conn->Connected())
    {
        conn->SetTcpNoDelay(true);
        conn->Send(owner_->message());
        owner_->OnConnect();
    }
    else
    {
        owner_->OnDisconnected(conn);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 7)
    {
        printf("Usage: client <host_ip> <port> <threads> <blocksize>");
        printf("<sessions> <time>\n");
    }
    else
    {
        std::stringstream ss;
        ss << std::this_thread::get_id();
        LOG_INFO << "pid = " << getpid() << ", tid = " << ss.str();
        Logger::SetLogLevel(Logger::LogLevel::kWarn);
        const char* ip = argv[1];
        uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
        int thread_num = atoi(argv[3]);
        int block_size = atoi(argv[4]);
        int session_count = atoi(argv[5]);
        int timeout = atoi(argv[6]);

        EventLoop loop;
        InetAddress server_addr(ip, port);
        Client client(&loop, server_addr, block_size, session_count, timeout, thread_num);
        loop.Loop();
    }
}