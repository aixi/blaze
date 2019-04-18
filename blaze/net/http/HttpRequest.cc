//
// Created by xi on 19-4-17.
//

#include <blaze/net/http/HttpRequest.h>

namespace blaze
{

namespace net
{

bool HttpRequest::SetMethod(const char* start, const char* end)
{
    assert(method_ == Method::kInvalid);
    std::string m(start, end);
    if (m == "GET")
    {
        method_ = Method::kGet;
    }
    else if (m == "POST")
    {
        method_ = Method::kPost;
    }
    else if (m == "HEAD")
    {
        method_ = Method::kHead;
    }
    else if (m == "PUT")
    {
        method_ = Method::kPut;
    }
    else if (m == "DELETE")
    {
        method_ = Method::kDelete;
    }
    else
    {
        method_ = Method::kInvalid;
    }
    return method_ != Method::kInvalid;
}

const char* HttpRequest::MethodString() const
{
    const char* result = "UNKNOWN";
    switch (method_)
    {
        case Method::kGet:
            result = "GET";
            break;
        case Method::kPost:
            result = "POST";
            break;
        case Method::kHead:
            result = "HEAD";
            break;
        case Method::kPut:
            result = "PUT";
            break;
        case Method::kDelete:
            result = "DELETE";
            break;
        case Method::kInvalid:
            result = "invalid";
            break;
    }
    return result;
}

// User-Agent: Mozilla/5.0 ......
void HttpRequest::AddHeader(const char* start, const char* colon, const char* end)
{
    std::string filed(start, colon);
    ++colon;
    while (colon < end && isspace(*colon))
    {
        ++colon;
    }
    std::string value(colon, end);
    // FIXME: use std::vector<char>::shrink_to_fit() ?
    while (!value.empty() && isspace(value[value.size() - 1]))
    {
        value.resize(value.size() - 1);
    }
    headers_[filed] = value;
}

// FIXME: std::optional ?
std::string HttpRequest::GetHeader(const std::string& field) const
{
    std::string result;
    auto iter = headers_.find(field);
    if (iter != headers_.end())
    {
        result = iter->second;
    }
    return result;
}

} // namespace net

} // namespace blaze