//
// Created by xi on 19-1-8.
//


#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <string>

#include <blaze/log/LogStream.h>

using namespace std;
using namespace blaze;

BOOST_AUTO_TEST_CASE(testLogStreamBools)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    BOOST_CHECK_EQUAL(buf.ToString(), std::string(""));
    os << true;
    BOOST_CHECK_EQUAL(buf.ToString(), std::string("1"));
    os << "\n";
    BOOST_CHECK_EQUAL(buf.ToString(), std::string("1\n"));
    os << false;
    BOOST_CHECK_EQUAL(buf.ToString(), std::string("1\n0"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegers)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    BOOST_CHECK_EQUAL(buf.ToString(), std::string(""));
    os << 1;
    BOOST_CHECK_EQUAL(buf.ToString(), std::string("1"));
    os << 0;
    BOOST_CHECK_EQUAL(buf.ToString(), std::string("10"));
    os << -1;
    BOOST_CHECK_EQUAL(buf.ToString(), std::string("10-1"));
    os.ResetBuffer();
    os << 0 << " " << 123 << 'x' << 0x64;
    BOOST_CHECK_EQUAL(buf.ToString(), std::string("0 123x100"));
}

BOOST_AUTO_TEST_CASE(testLogStreamIntegerLimits)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    os << -2147483647;
    BOOST_CHECK_EQUAL(buf.ToString(), string("-2147483647"));
    os << static_cast<int>(-2147483647 - 1);
    BOOST_CHECK_EQUAL(buf.ToString(), string("-2147483647-2147483648"));
    os << ' ';
    os << 2147483647;
    BOOST_CHECK_EQUAL(buf.ToString(), string("-2147483647-2147483648 2147483647"));
    os.ResetBuffer();

    os << std::numeric_limits<int16_t>::min();
    BOOST_CHECK_EQUAL(buf.ToString(), string("-32768"));
    os.ResetBuffer();

    os << std::numeric_limits<int16_t>::max();
    BOOST_CHECK_EQUAL(buf.ToString(), string("32767"));
    os.ResetBuffer();

    os << std::numeric_limits<uint16_t>::min();
    BOOST_CHECK_EQUAL(buf.ToString(), string("0"));
    os.ResetBuffer();

    os << std::numeric_limits<uint16_t>::max();
    BOOST_CHECK_EQUAL(buf.ToString(), string("65535"));
    os.ResetBuffer();

    os << std::numeric_limits<int32_t>::min();
    BOOST_CHECK_EQUAL(buf.ToString(), string("-2147483648"));
    os.ResetBuffer();

    os << std::numeric_limits<int32_t>::max();
    BOOST_CHECK_EQUAL(buf.ToString(), string("2147483647"));
    os.ResetBuffer();

    os << std::numeric_limits<uint32_t>::min();
    BOOST_CHECK_EQUAL(buf.ToString(), string("0"));
    os.ResetBuffer();

    os << std::numeric_limits<uint32_t>::max();
    BOOST_CHECK_EQUAL(buf.ToString(), string("4294967295"));
    os.ResetBuffer();

    os << std::numeric_limits<int64_t>::min();
    BOOST_CHECK_EQUAL(buf.ToString(), string("-9223372036854775808"));
    os.ResetBuffer();

    os << std::numeric_limits<int64_t>::max();
    BOOST_CHECK_EQUAL(buf.ToString(), string("9223372036854775807"));
    os.ResetBuffer();

    os << std::numeric_limits<uint64_t>::min();
    BOOST_CHECK_EQUAL(buf.ToString(), string("0"));
    os.ResetBuffer();

    os << std::numeric_limits<uint64_t>::max();
    BOOST_CHECK_EQUAL(buf.ToString(), string("18446744073709551615"));
    os.ResetBuffer();

    int16_t a = 0;
    int32_t b = 0;
    int64_t c = 0;
    os << a;
    os << b;
    os << c;
    BOOST_CHECK_EQUAL(buf.ToString(), string("000"));
}

BOOST_AUTO_TEST_CASE(testLogStreamFloats)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << 0.0;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0"));
    os.ResetBuffer();

    os << 1.0;
    BOOST_CHECK_EQUAL(buf.ToString(), string("1"));
    os.ResetBuffer();

    os << 0.1;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0.1"));
    os.ResetBuffer();

    os << 0.05;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0.05"));
    os.ResetBuffer();

    os << 0.15;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0.15"));
    os.ResetBuffer();

    double a = 0.1;
    os << a;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0.1"));
    os.ResetBuffer();

    double b = 0.05;
    os << b;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0.05"));
    os.ResetBuffer();

    double c = 0.15;
    os << c;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0.15"));
    os.ResetBuffer();

    os << a+b;
    BOOST_CHECK_EQUAL(buf.ToString(), string("0.15"));
    os.ResetBuffer();

    BOOST_CHECK(a+b != c);

    os << 1.23456789;
    BOOST_CHECK_EQUAL(buf.ToString(), string("1.23456789"));
    os.ResetBuffer();

    os << 1.234567;
    BOOST_CHECK_EQUAL(buf.ToString(), string("1.234567"));
    os.ResetBuffer();

    os << -123.456;
    BOOST_CHECK_EQUAL(buf.ToString(), string("-123.456"));
    os.ResetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamVoid)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << static_cast<void*>(0);
    BOOST_CHECK_EQUAL(buf.ToString(), string("0x0"));
    os.ResetBuffer();

    os << reinterpret_cast<void*>(8888);
    BOOST_CHECK_EQUAL(buf.ToString(), string("0x22B8"));
    os.ResetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamStrings)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << "Hello ";
    BOOST_CHECK_EQUAL(buf.ToString(), string("Hello "));

    string chenshuo = "Shuo Chen";
    os << chenshuo;
    BOOST_CHECK_EQUAL(buf.ToString(), string("Hello Shuo Chen"));
}

BOOST_AUTO_TEST_CASE(testLogStreamFmts)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();

    os << Fmt("%4d", 1);
    BOOST_CHECK_EQUAL(buf.ToString(), string("   1"));
    os.ResetBuffer();

    os << Fmt("%4.2f", 1.2);
    BOOST_CHECK_EQUAL(buf.ToString(), string("1.20"));
    os.ResetBuffer();

    os << Fmt("%4.2f", 1.2) << Fmt("%4d", 43);
    BOOST_CHECK_EQUAL(buf.ToString(), string("1.20  43"));
    os.ResetBuffer();
}

BOOST_AUTO_TEST_CASE(testLogStreamLong)
{
    LogStream os;
    const LogStream::Buffer& buf = os.buffer();
    for (int i = 0; i < 399; ++i)
    {
        os << "123456789 ";
        BOOST_CHECK_EQUAL(buf.Size(), 10 * (i+1));
        BOOST_CHECK_EQUAL(buf.Avail(), ::detail::kSmallBuffer - 10 * (i+1));
    }

    os << "abcdefghi ";
    BOOST_CHECK_EQUAL(buf.Size(), 4000);
    BOOST_CHECK_EQUAL(buf.Avail(), 96);

    os << "abcdefghi";
    BOOST_CHECK_EQUAL(buf.Size(), 4009);
    BOOST_CHECK_EQUAL(buf.Avail(), 87);
}
