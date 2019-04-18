//
// Created by xi on 19-4-17.
//

#ifndef BLAZE_HTTPRESPONSE_H
#define BLAZE_HTTPRESPONSE_H

#include <map>

#include <blaze/utils/copyable.h>
#include <blaze/utils/Types.h>

namespace blaze
{
namespace net
{

class Buffer;

class HttpResponse : public copyable
{
public:
    enum class HttpStatusCode : int
    {
        kUnknown,
        k200Ok = 200,
        k301MovePermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
        // TODO
    };

    explicit HttpResponse(bool close) :
        status_code_(HttpStatusCode::kUnknown),
        close_connection_(close)
    {}

    void SetStatusCode(HttpStatusCode code)
    {
        status_code_ = code;
    }

    void SetStatusMessage(const std::string& message)
    {
        status_message_ = message;
    }

    void SetCloseConnection(bool on)
    {
        close_connection_ = on;
    }

    bool IsCloseConnection() const
    {
        return close_connection_;
    }

    void SetContentType(const std::string& content_type)
    {
        AddHeader("Content-Type", content_type);
    }

    // FIXME: use std::string_view
    void AddHeader(const std::string& key, const std::string& value)
    {
        headers_[key] = value;
    }

    // FIXME: use std::string_view
    void SetBody(const std::string& body)
    {
        body_ = body;
    }

    void AppendToBuffer(Buffer* output) const;

private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode status_code_;
    // FIXME: http version ?
    std::string status_message_;
    bool close_connection_;
    std::string body_;
};

} // namespace net
} // namespace blaze

#endif //BLAZE_HTTPRESPONSE_H
