//
// Created by xi on 19-4-17.
//

#include <stdio.h>
#include <blaze/net/Buffer.h>
#include <blaze/net/http/HttpResponse.h>

using namespace blaze;
using namespace blaze::net;

void HttpResponse::AppendToBuffer(Buffer* output) const
{
    char buf[32];
    snprintf(buf, sizeof(buf), "HTTP1.1 %d", status_code_);
    output->Append(buf);
    output->Append(status_message_);
    output->Append("\r\n");
    if (close_connection_)
    {
        output->Append("Connection: close\r\n");
    }
    else
    {
        snprintf(buf, sizeof(buf), "Content-Length: %zd\r\n", body_.size());
        output->Append(buf);
        output->Append("Connection: Keep-Alive\r\n");
    }
    for (const std::pair<const std::string, std::string>& header : headers_)
    {
        output->Append(header.first);
        output->Append(": ");
        output->Append(header.second);
        output->Append("\r\n");
    }
    output->Append("\r\n");
    output->Append(body_);
}