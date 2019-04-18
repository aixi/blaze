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