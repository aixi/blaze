//
// Created by xi on 19-4-17.
//

#ifndef BLAZE_HTTPCONTEXT_H
#define BLAZE_HTTPCONTEXT_H

#include <blaze/utils/copyable.h>
#include <blaze/net/http/HttpRequest.h>

namespace blaze
{

namespace net
{

class Buffer;

class HttpContext : public copyable
{
public:
    enum class HttpRequestParseState
    {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll
    };

    // default copy-ctor, dtor and assignment are OK

    HttpContext() :
        state_(HttpRequestParseState::kExpectRequestLine)
    {}

    // return false if any error
    bool ParseRequest(Buffer* buf, Timestamp receive_time);

    bool GotAll() const
    {
        return state_ == HttpRequestParseState::kGotAll;
    }

    void Reset()
    {
        state_ = HttpRequestParseState::kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    const HttpRequest& GetRequest() const
    {
        return request_;
    }

    HttpRequest& GetRequest()
    {
        return request_;
    }

private:
    bool ProcessRequestLine(const char* begin, const char* end);

private:
    HttpRequestParseState state_;
    HttpRequest request_;
};

} // namespace net

} // namespace blaze

#endif //BLAZE_HTTPCONTEXT_H
