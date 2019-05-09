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