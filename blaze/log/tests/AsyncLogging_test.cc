//
// Created by xi on 19-1-22.
//

#include <stdio.h>
#include <sys/resource.h> // setrlimit
#include <unistd.h>

#include <blaze/log/Logging.h>
#include <blaze/utils/Timestamp.h>
#include <blaze/log/AsyncLogging.h>

off_t kRollSize = 500 * 1000 * 1000;

blaze::AsyncLogging* g_asyncLog = nullptr;

void AsyncOutput(const char* msg, int len)
{
    g_asyncLog->Append(msg, len);
}

void Bench(bool long_log)
{
    blaze::Logger::SetOutput(AsyncOutput);
    int cnt = 0;
    const int kBatch = 1024;
    std::string empty(" ");
    std::string long_str(3000, 'X');
    long_str += " ";
    for (int i = 0; i < 30; ++i)
    {
        blaze::Timestamp start = blaze::Timestamp::Now();
        for (int j = 0; j < kBatch; ++j)
        {
            LOG_INFO << "Hello 0123456789" << " abcdefghijklmnopqrstuvwxyz "
                << (long_log ? long_str : empty) << cnt;
            ++cnt;
        }
        blaze::Timestamp end = blaze::Timestamp::Now();
        printf("%f\n", blaze::TimeDifference(start, end) * 1000'000 / kBatch);
        struct timespec ts = {0, 500 * 1000 * 1000};
        nanosleep(&ts, nullptr);
    }
}

int main(int argc, char* argv[])
{
    // https://www.cnblogs.com/niocai/archive/2012/04/01/2428128.html
    // set virtual memory to 2GB
    {
        size_t kOneGB = (2 << 30);
        rlimit rl = {2 * kOneGB, 2 * kOneGB};
        setrlimit(RLIMIT_AS, &rl);
    }
    printf("pid=%d\n", getpid());
    char name[256] = {0};
    strncpy(name, argv[0], sizeof(name) - 1);
    blaze::AsyncLogging log(::basename(name), kRollSize);
    log.Start();
    g_asyncLog = &log;
    bool long_log = argc > 1;
    Bench(long_log);
    return 0;
}