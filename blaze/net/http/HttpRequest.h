//
// Created by xi on 19-4-17.
//

#ifndef BLAZE_HTTPREQUEST_H
#define BLAZE_HTTPREQUEST_H

#include <stdio.h>
#include <assert.h>
#include <map>
#include <blaze/utils/copyable.h>
#include <blaze/utils/Timestamp.h>
#include <blaze/utils/Types.h>

namespace blaze
{
namespace net
{

class HttpRequest  : public copyable
{
public:
    enum class Method
    {
        kInvalid,
        kGet,
        kPost,
        kHead,
        kPut,
        kDelete,
    };

    enum class Version
    {
        kUnknown,
        kHttp10,
        kHttp11
    };

    // default copy-ctor, dtor and assignment are OK

    HttpRequest() :
        method_(Method::kInvalid),
        version_(Version::kUnknown)
    {

    }

    void SetVersion(Version v)
    {
        version_ = v;
    }

    Version GetVersion() const
    {
        return version_;
    }

    bool SetMethod(const char* start, const char* end);

    Method GetMethod() const
    {
        return method_;
    }

    const char* MethodString() const;

    void SetPath(const char* start, const char* end)
    {
        path_.assign(start, end);
    }

    const std::string& GetPath() const
    {
        return path_;
    }

    void SetQuery(const char* start, const char* end)
    {
        query_.assign(start, end);
    }

    const std::string& GetQuery() const
    {
        return query_;
    }

    void SetReceiveTime(Timestamp t)
    {
        receive_time_ = t;
    }

    Timestamp GetReceiveTime() const
    {
        return receive_time_;
    }

    // User-Agent: Mozilla/5.0 ......
    void AddHeader(const char* start, const char* colon, const char* end);

    // FIXME: return type std::optional ?
    std::string GetHeader(const std::string& field) const;

    void swap(HttpRequest& other) noexcept
    {
        std::swap(method_, other.method_);
        std::swap(version_, other.version_);
        path_.swap(other.path_);
        query_.swap(other.query_);
        receive_time_.Swap(other.receive_time_);
        headers_.swap(other.headers_);
    }

    const std::map<std::string, std::string>& GetHeaders() const
    {
        return headers_;
    }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    Timestamp receive_time_;
    std::map<std::string, std::string> headers_;
};

} // namespace net

} // namespace blaze

#endif //BLAZE_HTTPREQUEST_H
