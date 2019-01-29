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
}