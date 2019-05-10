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
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

include_directories("${PROJECT_SOURCE_DIR}")

find_package(Boost REQUIRED)
find_package(Protobuf)
find_package(ZLIB)
find_package(Threads)

if (BOOST_FOUND)
    message(STATUS "found Boost")
endif()

if (PROTOBUF_FOUND)
    message(STATUS "found Protobuf")
endif()

if (ZLIB_FOUND)
    message(STATUS "found zlib")
endif()

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
        blaze/concurrent/ThreadGuard.cc
        blaze/net/ZlibStream.cc)

set(CONCURRENT_HEADERS
        blaze/concurrent/BlockingQueue.h
        blaze/concurrent/BoundedBlockingQueue.h
        blaze/concurrent/CountDownLatch.h
        blaze/concurrent/ThreadGuard.h
        blaze/concurrent/ThreadPool.h)

set(LOG_HEADERS
        blaze/log/AsyncLogging.h
        blaze/log/LogFile.h
        blaze/log/Logging.h
        blaze/log/LogStream.h)

set(NET_HEADERS
        blaze/net/Acceptor.h
        blaze/net/Buffer.h
        blaze/net/Callbacks.h
        blaze/net/Channel.h
        blaze/net/Connector.h
        blaze/net/Endian.h
        blaze/net/EventLoop.h
        blaze/net/EventLoopThread.h
        blaze/net/EventLoopThreadPool.h
        blaze/net/InetAddress.h
        blaze/net/Poller.h
        blaze/net/Socket.h
        blaze/net/SocketsOps.h
        blaze/net/TcpClient.h
        blaze/net/TcpConnection.h
        blaze/net/TcpServer.h
        blaze/net/Timer.h
        blaze/net/TimerId.h
        blaze/net/TimerQueue.h
        blaze/net/ZlibStream.h)

set(UTILS_HEADERS
        blaze/utils/copyable.h
        blaze/utils/Exception.h
        blaze/utils/FileUtil.h
        blaze/utils/noncopyable.h
        blaze/utils/ProcessInfo.h
        blaze/utils/Singleton.h
        blaze/utils/Timestamp.h
        blaze/utils/Types.h
        blaze/utils/WeakCallback.h)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(blaze/concurrent/tests)
add_subdirectory(blaze/log/tests)
add_subdirectory(blaze/net/tests)
add_subdirectory(blaze/utils/tests)
add_subdirectory(blaze/net/http)
add_subdirectory(blaze/net/http/tests)
add_subdirectory(examples/chat)
add_subdirectory(examples/daytime)
add_subdirectory(examples/echo)
add_subdirectory(examples/pingpong)

add_library(blaze STATIC ${SOURCE_FILES})
target_link_libraries(blaze pthread)

install(FILES ${CONCURRENT_HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/include/concurrent)
install(FILES ${LOG_HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/include/log)
install(FILES ${NET_HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/include/net)
install(FILES ${UTILS_HEADERS} DESTINATION ${PROJECT_BINARY_DIR}/include/utils)