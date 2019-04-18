//
// Created by xi on 19-4-17.
//

#include <blaze/log/Logging.h>
#include <blaze/net/TcpConnection.h>
#include <blaze/net/http/HttpContext.h>
#include <blaze/net/http/HttpRequest.h>
#include <blaze/net/http/HttpResponse.h>
#include <blaze/net/http/HttpServer.h>

namespace blaze
{
namespace net
{
namespace detail
{

void DefaultHttpCallback(const HttpRequest& request, HttpResponse* response)
{
    response->SetStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
    response->SetStatusMessage("Not Found");
    response->SetCloseConnection(true);
}

} // namespace detail

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& server_addr,
                       std::string_view name,
                       TcpServer::Option option) :
    server_(loop, server_addr, name, option),
    http_callback_(detail::DefaultHttpCallback)
{
    server_.SetConnectionCallback(std::bind(&HttpServer::OnConnection, this, _1));
    server_.SetMessageCallback(std::bind(&HttpServer::OnMessage, this, _1, _2, _3));
}

void HttpServer::Start()
{
    LOG_WARN << "HttpServer[" << server_.GetName()
             << "] starts listening on " << server_.IpPort();
    server_.Start();
}

void HttpServer::OnConnection(const TcpConnectionPtr& conn)
{
    if (conn->Connected())
    {
        conn->SetContext(HttpContext());
    }
}

void HttpServer::OnMessage(const TcpConnectionPtr& conn,
                           Buffer* buf,
                           Timestamp receive_time)
{
    HttpContext* context = std::any_cast<HttpContext>(conn->GetMutableContext());
    if (!context->ParseRequest(buf, receive_time))
    {
        conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->Shutdown();
    }
    if (context->GotAll())
    {
        OnRequest(conn, context->GetRequest());
        context->Reset();
    }
}

void HttpServer::OnRequest(const TcpConnectionPtr& conn, const HttpRequest& request)
{
    const std::string& connection = request.GetHeader("Connection");
    bool close = connection == "close" ||
        (request.GetVersion() == HttpRequest::Version::kHttp10 && connection != "Keep-Alive");
    HttpResponse response(close);
    http_callback_(request, &response);
    Buffer buf;
    response.AppendToBuffer(&buf);
    conn->Send(&buf);
    if (response.IsCloseConnection())
    {
        conn->Shutdown();
    }
}

} // namespace net
} // namespace blaze
