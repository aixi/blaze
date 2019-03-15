cmake_minimum_required(VERSION 3.0)

project(blaze)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CXX_FLAGS
        -g
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wold-style-cast
        -Wsequence-point
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wthread-safety
        -Wwrite-strings
        -march=native
        -std=c++17
        -Wno-unused-parameter
        -Wno-null-dereference
        -Wno-sign-conversion
        -Wno-unused-private-field
        -fmodules)

if(CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

include_directories("${PROJECT_SOURCE_DIR}")

set(SOURCE_FILES
        blaze/log/LogStream.cc
        blaze/utils/FileUtil.cc
        blaze/log/Logging.cc
        blaze/utils/Timestamp.cc
        blaze/log/LogFile.cc
        blaze/utils/ProcessInfo.cc
        blaze/concurrent/ThreadPool.cc
        blaze/utils/Exception.cc
        blaze/concurrent/CountDownLatch.cc
        blaze/log/AsyncLogging.cc
        blaze/net/Buffer.cc
        blaze/net/SocketsOps.cc
        blaze/net/InetAddress.cc
        blaze/net/Socket.cc
        blaze/net/Channel.cc
        blaze/net/poller/PollPoller.cc
        blaze/net/EventLoop.cc
        blaze/net/Poller.cc
        blaze/net/poller/DefaultPoller.cc
        blaze/net/TimerQueue.cc
        blaze/net/Timer.cc
        blaze/net/EventLoopThread.cc
        blaze/net/Acceptor.cc
        blaze/net/TcpConnection.cc
        blaze/net/TcpServer.cc
        blaze/net/poller/EPollPoller.cc
        blaze/net/EventLoopThreadPool.cc
        blaze/net/Connector.cc
        blaze/net/TcpClient.cc
        blaze/concurrent/ThreadGuard.cc)

add_library(blaze STATIC ${SOURCE_FILES})
target_link_libraries(blaze pthread)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_executable(blaze_simple examples/simple.cc)
target_link_libraries(blaze_simple blaze)

add_executable(LogFile_test blaze/log/tests/LogFile_test.cc)
target_link_libraries(LogFile_test blaze)

add_executable(LogStream_unittest blaze/log/tests/LogStream_unittest.cc)
target_link_libraries(LogStream_unittest blaze boost_unit_test_framework)
add_test(NAME LogStream_unittest COMMAND LogStream_unittest)

add_executable(Buffer_unittest blaze/net/tests/Buffer_unittest.cc)
target_link_libraries(Buffer_unittest blaze boost_unit_test_framework)
add_test(NAME Buffer_unittest COMMAND Buffer_unittest)

add_executable(Singleton_test blaze/utils/tests/Singleton_test.cc)
target_link_libraries(Singleton_test blaze)

add_executable(Timestamp_test blaze/utils/tests/Timestamp_test.cc)
target_link_libraries(Timestamp_test blaze)

add_executable(Exception_test blaze/utils/tests/Exception_test.cc)
target_link_libraries(Exception_test blaze)

add_executable(ThreadPool_test blaze/concurrent/tests/ThreadPool_test.cc)
target_link_libraries(ThreadPool_test blaze)

add_executable(Logging_test blaze/log/tests/Logging_test.cc)
target_link_libraries(Logging_test blaze)

add_executable(AsyncLogging_test blaze/log/tests/AsyncLogging_test.cc)
target_link_libraries(AsyncLogging_test blaze)

add_executable(EventLoopThread_test blaze/net/tests/EventLoopThread_test.cc)
target_link_libraries(EventLoopThread_test blaze)

add_executable(EchoServer_test blaze/net/tests/EchoServer_test.cc)
target_link_libraries(EchoServer_test blaze)

add_executable(EventLoopThreadPool_test blaze/net/tests/EventLoopThreadPool_unittest.cc)
target_link_libraries(EventLoopThreadPool_test blaze)

add_executable(Channel_test blaze/net/tests/Channel_test.cc)
target_link_libraries(Channel_test blaze)

add_executable(EventLoop_test blaze/net/tests/EventLoop_test.cc)
target_link_libraries(EventLoop_test blaze)

#add_executable(EchoClient_test blaze/net/tests/EchoClient_test.cc)
#target_link_libraries(EchoClient_test blaze)
#
#add_executable(TcpClient_reg1 blaze/net/tests/TcpClient_reg1.cc)
#target_link_libraries(TcpClient_reg1 blaze)
#
#add_executable(TcpClient_reg2 blaze/net/tests/TcpClient_reg1.cc)
#target_link_libraries(TcpClient_reg2 blaze)
#
#add_executable(TcpClient_reg3 blaze/net/tests/TcpClient_reg1.cc)
#target_link_libraries(TcpClient_reg3 blaze)

add_executable(InetAddress_unittest blaze/net/tests/InetAddress_unittest.cc)
target_link_libraries(InetAddress_unittest blaze boost_unit_test_framework)
add_test(NAME InetAddress_unittest COMMAND InetAddress_unittest)

add_executable(TimerQueue_test blaze/net/tests/TimerQueue_test.cc)
target_link_libraries(TimerQueue_test blaze)

add_executable(pingpong_client examples/pingpong/client.cc)
target_link_libraries(pingpong_client blaze)

add_executable(pingpong_server examples/pingpong/server.cc)
target_link_libraries(pingpong_server blaze)

#add_executable(pingpong_bench examples/pingpong/bench.cc)
#target_link_libraries(pingpong_bench blaze)

add_executable(BlockingQueue_test blaze/concurrent/tests/BlockingQueue_test.cc)
target_link_libraries(BlockingQueue_test blaze)

add_executable(chat_client examples/chat/client.cc)
target_link_libraries(chat_client blaze)

add_executable(chat_server examples/chat/server.cc)
target_link_libraries(chat_server blaze)