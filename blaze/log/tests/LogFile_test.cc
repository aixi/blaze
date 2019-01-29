//
// Created by xi on 19-1-9.
//

#include <unistd.h>

#include <blaze/log/LogFile.h>
#include <blaze/log/Logging.h>

using namespace blaze;

std::unique_ptr<LogFile> g_logFile;

void OutputFunc(const char* msg, int len)
{
    g_logFile->Append(msg, len);
}

void FlushFunc()
{
    g_logFile->Flush();
}

int main(int argc, char* argv[])
{
    char name[256] = {0};
    strncpy(name, argv[0], sizeof(name) - 1);
    g_logFile = std::make_unique<LogFile>(::basename(name), 200 * 1000);
    Logger::SetOutput(OutputFunc);
    Logger::SetFlush(FlushFunc);
    std::string line = "Logline Test---------------------------------------------------------Logline tests";
    for (int i = 0; i < 10000; ++i)
    {
        LOG_INFO << line << i;
        usleep(1000);
    }
}