//
// Created by xi on 19-1-21.
//

#include <stdio.h>
#include <unistd.h>

#include <blaze/concurrent/ThreadPool.h>
#include <blaze/log/Logging.h>
#include <blaze/log/LogFile.h>

int g_total;
FILE* g_file;
std::unique_ptr<blaze::LogFile> g_logFile;

using namespace blaze;

void DummyOutput(const char* msg, int len)
{
    g_total += len;
    if (g_file)
    {
        fwrite(msg, 1, len, g_file);
    }
    else if (g_logFile)
    {
        g_logFile->Append(msg, len);
    }
}

void Bench(const char* type)
{
    Logger::SetOutput(DummyOutput);
    blaze::Timestamp start(blaze::Timestamp::Now());
    g_total = 0;
    int n = 1000 * 1000;
    const bool kLongLog = false;
    std::string empty(" ");
    std::string long_str(3000, 'X');
    long_str.push_back(' ');
    for (int i = 0; i < n; ++i)
    {
        LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz"
                 << (kLongLog ? long_str : empty)
                 << i;
    }
    blaze::Timestamp end(blaze::Timestamp::Now());
    double seconds = blaze::TimeDifference(start, end);
    printf("%12s: %f seconds, %d bytes, %10.2f msg/s, %.2f MiB/s\n",
            type, seconds, g_total, n / seconds, g_total / seconds);
}

void LogInThread()
{
    LOG_INFO << "LogInThread()";
    usleep(1000);
}

int main()
{
    getpid(); // for ltrace and strace
    blaze::ThreadPool pool("ThreadPool");
    pool.Start(5);
    pool.Run(LogInThread);
    pool.Run(LogInThread);
    pool.Run(LogInThread);
    pool.Run(LogInThread);
    pool.Run(LogInThread);

    LOG_TRACE << "trace";
    LOG_DEBUG << "debug";
    LOG_INFO << "Hello";
    LOG_WARN << "World";
    LOG_ERROR << "Error";
    LOG_INFO << sizeof(Logger);
    LOG_INFO << sizeof(blaze::LogStream);
    LOG_INFO << sizeof(blaze::Fmt);
    LOG_INFO << sizeof(blaze::LogStream::Buffer);

    sleep(1);
    Bench("nop");

    char buffer[64 * 1024];

    g_file = fopen("/dev/null", "w");
    setbuffer(g_file, buffer, sizeof(buffer));
    Bench("/dev/null");
    fclose(g_file);

    g_file = fopen("/tmp/log", "w");
    setbuffer(g_file, buffer, sizeof(buffer));
    Bench("/tmp/log");
    fclose(g_file);

    g_file = nullptr;
    g_logFile.reset(new blaze::LogFile("test_log_st", 500 * 1000 * 1000, false));
    Bench("test_log_st");

    g_logFile.reset(new blaze::LogFile("test_log_mt", 500 * 1000 * 1000, true));
    Bench("test_log_mt");
    g_logFile.reset();

    return 0;
}