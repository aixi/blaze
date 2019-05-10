//
// Created by xi on 19-5-9.
//

#include <stdio.h>

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <blaze/log/Logging.h>
#include <blaze/net/ZlibStream.h>

using namespace blaze;
using namespace blaze::net;

BOOST_AUTO_TEST_CASE(testZlibOutputStream)
{
    Buffer output;
    {
        ZlibOutputStream stream(&output);
        BOOST_CHECK_EQUAL(output.ReadableBytes(), 0);
    }
    BOOST_CHECK_EQUAL(output.ReadableBytes(), 8);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream1)
{
    Buffer output;
    ZlibOutputStream stream(&output);
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_OK);
    stream.Finish();
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream2)
{
    Buffer output;
    ZlibOutputStream stream(&output);
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_OK);
    BOOST_CHECK(stream.Write("0123456789012345678901234567890123456789"));
    stream.Finish();
    printf("%zd\n", output.ReadableBytes());
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream3)
{
    Buffer output;
    ZlibOutputStream stream(&output);
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_OK);
    for (int i = 0; i < 1024; ++i)
    {
        BOOST_CHECK(stream.Write("0123456789012345678901234567890123456789"));
    }
    stream.Finish();
    printf("total %zd\n", output.ReadableBytes());
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream4)
{
    Buffer output;
    ZlibOutputStream stream(&output);
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_OK);
    std::string input;
    std::string_view temp("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-");
    for (int i = 0; i < 32768; ++i)
    {
        input += temp[rand() % 64];
    }
    for (int i = 0; i < 10; ++i)
    {
        BOOST_CHECK(stream.Write(input));
    }
    stream.Finish();
    printf("total %zd\n", output.ReadableBytes());
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_STREAM_END);
}

BOOST_AUTO_TEST_CASE(testZlibOutputStream5)
{
    Buffer output;
    ZlibOutputStream stream(&output);
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_OK);
    std::string input(1024 * 1024, '_');
    for (int i = 0; i < 64; ++i)
    {
        BOOST_CHECK(stream.Write(input));
    }
    printf("buf size %d\n", stream.InternalOutputBufferSize());
    LOG_INFO << "total_in" << stream.InputBytes();
    LOG_INFO << "totla_out" << stream.OutputBytes();
    stream.Finish();
    printf("total %zd\n", output.ReadableBytes());
    BOOST_CHECK_EQUAL(stream.ZlibErrorCode(), Z_STREAM_END);
}