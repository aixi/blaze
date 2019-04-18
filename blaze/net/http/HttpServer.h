//
// Created by xi on 19-4-17.
//

#ifndef BLAZE_HTTPSERVER_H
#define BLAZE_HTTPSERVER_H

#include <string_view>
#include <blaze/utils/Types.h>
#include <blaze/net/TcpServer.h>

namespace blaze
{
namespace net
{

class HttpRequest;
class HttpResponse;

// used for report status of a program,
// not fully HTTP 1.1 compliant, can communicate with HttpClient and Web browser.
// It is synchronous, just like Java Servlet,

class HttpServer
{
public:

    using HttpCallback = std::function<void (const HttpRequest&, HttpResponse*)>;

    HttpServer(EventLoop* loop,
               const InetAddress& server_addr,
               std::string_view name,
               TcpServer::Option option = TcpServer::Option::kNoReusePort);

    EventLoop* GetLoop() const
    {
        return server_.GetLoop();
    }

    void SetHttpCallback(HttpCallback cb)
    {
        http_callback_ = std::move(cb);
    }

    void SetThreadNum(int num)
    {
        server_.SetThreadNum(num);
    }

    void Start();

    DISABLE_COPY_AND_ASSIGN(HttpServer);

private:
    void OnConnection(const TcpConnectionPtr& conn);

    void OnMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receive_time);

    void OnRequest(const TcpConnectionPtr& conn, const HttpRequest& request);

private:
    TcpServer server_;
    HttpCallback http_callback_;
};

} // namespace net
} // namespace blaze

#endif //BLAZE_HTTPSERVER_H
