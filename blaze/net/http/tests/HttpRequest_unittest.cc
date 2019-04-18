//
// Created by xi on 19-4-18.
//

#include <blaze/net/Buffer.h>
#include <blaze/net/http/HttpContext.h>

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using namespace blaze;
using namespace blaze::net;

BOOST_AUTO_TEST_CASE(testParseRequestAllInOne)
{
    HttpContext context;
    Buffer input;
    input.Append("GET /index.html HTTP/1.1\r\nHost: www.aixi.com\r\n\r\n");
    BOOST_CHECK(context.ParseRequest(&input, Timestamp::Now()));
    BOOST_CHECK(context.GotAll());
    const HttpRequest& request = context.GetRequest();
    BOOST_CHECK(request.GetMethod() == HttpRequest::Method::kGet);
    BOOST_CHECK_EQUAL(request.GetPath(), std::string("/index.html"));
    BOOST_CHECK(request.GetVersion() == HttpRequest::Version::kHttp11);
    BOOST_CHECK_EQUAL(request.GetHeader("Host"), std::string("www.aixi.com"));
    BOOST_CHECK_EQUAL(request.GetHeader("User-Agent"), std::string(""));
}

BOOST_AUTO_TEST_CASE(testParseRequestInTwoPieces)
{
    std::string all("GET /index.html HTTP/1.1\r\nHost: www.aixi.com\r\n\r\n");
    for (size_t len = 0; len < all.size(); ++len)
    {
        HttpContext context;
        Buffer input;
        input.Append(all.c_str(), len);
        BOOST_CHECK(context.ParseRequest(&input, Timestamp::Now()));
        BOOST_CHECK(!context.GotAll());

        size_t len2 = all.size() - len;
        input.Append(all.c_str() + len, len2);
        BOOST_CHECK(context.ParseRequest(&input, Timestamp::Now()));
        BOOST_CHECK(context.GotAll());
        const HttpRequest& request = context.GetRequest();
        BOOST_CHECK(request.GetMethod() == HttpRequest::Method::kGet);
        BOOST_CHECK_EQUAL(request.GetPath(), std::string("/index.html"));
        BOOST_CHECK(request.GetVersion() == HttpRequest::Version::kHttp11);
        BOOST_CHECK_EQUAL(request.GetHeader("Host"), std::string("www.aixi.com"));
        BOOST_CHECK_EQUAL(request.GetHeader("User-Agent"), std::string(""));
    }
}

BOOST_AUTO_TEST_CASE(testParseRequestEmptyHeaderValue)
{
    HttpContext context;
    Buffer input;
    input.Append("GET /index.html HTTP/1.1\r\nHost: www.aixi.com\r\nUser-Agent: \r\nAccept-Encoding: \r\n\r\n");
    BOOST_CHECK(context.ParseRequest(&input, Timestamp::Now()));
    BOOST_CHECK(context.GotAll());
    const HttpRequest& request = context.GetRequest();
    BOOST_CHECK(request.GetMethod() == HttpRequest::Method::kGet);
    BOOST_CHECK_EQUAL(request.GetPath(), std::string("/index.html"));
    BOOST_CHECK(request.GetVersion() == HttpRequest::Version::kHttp11);
    BOOST_CHECK_EQUAL(request.GetHeader("Host"), std::string("www.aixi.com"));
    BOOST_CHECK_EQUAL(request.GetHeader("User-Agent"), std::string(""));
    BOOST_CHECK_EQUAL(request.GetHeader("Accept-Encoding"), std::string(""));
}