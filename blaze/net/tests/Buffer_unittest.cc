//
// Created by xi on 19-1-29.
//

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <blaze/net/Buffer.h>

using namespace blaze;
using namespace blaze::net;

BOOST_AUTO_TEST_CASE(testBufferAppendRetrieve)
{
    Buffer buf;
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);

    std::string str(200, 'x');
    buf.Append(str);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), str.size());
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - str.size());
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);

    std::string str2(buf.RetrieveAsString(50));
    BOOST_CHECK_EQUAL(str2.size(), 50);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), str.size() - str2.size());
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - str.size());
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + str2.size());
    BOOST_CHECK_EQUAL(str2, std::string(50, 'x'));

    buf.Append(str);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 2 * str.size() - str2.size());
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 2 * str.size());
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + str2.size());

    std::string str3(buf.RetrieveAllAsString());
    BOOST_CHECK_EQUAL(str3.size(), 350);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
    BOOST_CHECK_EQUAL(str3, std::string(350, 'x'));
}

BOOST_AUTO_TEST_CASE(testBufferGrow)
{
    Buffer buf;
    buf.Append(std::string(400, 'y'));
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 400);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 400);

    buf.Retrieve(50);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 350);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 400);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + 50);

    buf.Append(std::string(1000, 'z'));
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 1350);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + 50);

    buf.RetrieveAll();
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), 1400);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
}

// it will move readable bytes to front

BOOST_AUTO_TEST_CASE(testBufferDataTransfer)
{
    Buffer buf;
    buf.Append(std::string(800, 'y'));
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 800);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 800);

    buf.Retrieve(500);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 300);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 800);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + 500);

    buf.Append(std::string(300, 'z'));
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 600);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 600);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferShrink)
{
    Buffer buf;
    buf.Append(std::string(2000, 'y'));
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 2000);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);

    buf.Retrieve(1500);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 500);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), 0);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend + 1500);

    buf.Shrink(0);
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 500);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 500);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
    BOOST_CHECK_EQUAL(buf.RetrieveAllAsString(), std::string(500, 'y'));
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);
}

BOOST_AUTO_TEST_CASE(testBufferPrepend)
{
    Buffer buf;
    buf.Append(std::string(200, 'y'));
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 200);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 200);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend);

    int x = 0;
    buf.Prepend(&x, sizeof(x));
    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 204);
    BOOST_CHECK_EQUAL(buf.WritableBytes(), Buffer::kInitialSize - 200);
    BOOST_CHECK_EQUAL(buf.PrependableBytes(), Buffer::kCheapPrepend - 4);
}

BOOST_AUTO_TEST_CASE(testBufferReadInt)
{
    Buffer buf;
    buf.Append("HTTP");

    BOOST_CHECK_EQUAL(buf.ReadableBytes(), 4);
    BOOST_CHECK_EQUAL(buf.PeekInt8(), 'H');
//    int top16 = buf.PeekInt16();
    // TODO
}

BOOST_AUTO_TEST_CASE(testBufferFindEOL)
{
    Buffer buf;
    buf.Append(std::string(100'000, 'x'));
    const char* null = nullptr;
    BOOST_CHECK_EQUAL(buf.FindEOL(), null);
    BOOST_CHECK_EQUAL(buf.FindEOL(buf.BeginRead() + 90'000), null);
}

void output(Buffer&& buf, const void* inner)
{
    Buffer new_buf(std::move(buf));
    BOOST_CHECK_EQUAL(inner, new_buf.BeginRead());
}

// NOTE: failed in g++ 4.4

BOOST_AUTO_TEST_CASE(testMove)
{
    Buffer buf;
    buf.Append("blaze", 5);
    const void* inner = buf.BeginRead();
    output(std::move(buf), inner);
}
