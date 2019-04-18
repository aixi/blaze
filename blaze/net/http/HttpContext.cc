//
// Created by xi on 19-4-17.
//

#include <blaze/net/Buffer.h>
#include <blaze/net/http/HttpContext.h>

namespace blaze
{

namespace net
{

// return false if any error
bool HttpContext::ParseRequest(Buffer* buf, Timestamp receive_time)
{
    bool ok = true;
    bool has_more = true;
    while (has_more)
    {
        if (state_ == HttpRequestParseState::kExpectRequestLine)
        {
            // NOTE: Accept-Language: zh-CN \r\n
            const char* crlf = buf->FindCRLF();
            if (crlf)
            {
                ok = ProcessRequestLine(buf->BeginRead(), crlf);
                if (ok)
                {
                    request_.SetReceiveTime(receive_time);
                    buf->RetrieveUntil(crlf + 2);
                    state_ = HttpRequestParseState::kExpectHeaders;
                }
                else
                {
                    has_more = false;
                }
            }
            else
            {
                has_more = false;
            }
        }
        else if (state_ == HttpRequestParseState::kExpectHeaders)
        {
            const char* crlf = buf->FindCRLF();
            if (crlf)
            {
                const char* colon = std::find(buf->BeginRead(), crlf, ':');
                if (colon != crlf)
                {
                    request_.AddHeader(buf->BeginRead(), colon, crlf);
                }
                else
                {
                    // empty line, end of header
                    // FIXME:
                    state_ = HttpRequestParseState::kGotAll;
                    has_more = false;
                }
                buf->RetrieveUntil(crlf + 2);
            }
            else
            {
                has_more = false;
            }
        }
        else if (state_ == HttpRequestParseState::kExpectBody)
        {
            // TODO: handle body
        }
    }
    return ok;
}

bool HttpContext::ProcessRequestLine(const char* begin, const char* end)
{
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.SetMethod(start, space))
    {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end)
        {
            const char* question = std::find(start, space, '?');
            if (question == end)
            {
                request_.SetPath(start, space);
            }
            else // there is query in url
            {
                request_.SetPath(start, question);
                request_.SetQuery(question, space);
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed)
            {
                char ch = *(end - 1);
                if (ch == '1')
                {
                    request_.SetVersion(HttpRequest::Version::kHttp11);
                }
                else if (ch == '0')
                {
                    request_.SetVersion(HttpRequest::Version::kHttp10);
                }
                else
                {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

}

}
