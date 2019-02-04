cmake_minimum_required(VERSION 3.0)

project(blaze)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

set(CXX_FLAGS
        -g
        -Wall
        -Wextra
        -Werror
        -Wconversion
        -Wno-unused-parameter
        -Wold-style-cast
        -Wsequence-point
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wthread-safety
        -Wwrite-strings
        -march=native
        -std=c++17
        -Wno-null-dereference
        -Wno-sign-conversion
        -Wno-unused-private-field
        -Wthread-safety
        -fmodules)

if(CMAKE_BUILD_BITS EQUAL 32)
    list(APPEND CXX_FLAGS "-m32")
endif()

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
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
        blaze/net/TcpServer.cc)

add_library(blaze STATIC ${SOURCE_FILES})
target_link_libraries(blaze pthread)

add_executable(FileUtil_test blaze/utils/tests/FileUtil_test.cc)
target_link_libraries(FileUtil_test blaze)

add_executable(blaze_simple examples/main.cc)
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
