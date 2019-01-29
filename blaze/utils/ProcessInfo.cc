//
// Created by xi on 19-1-18.
//
#include <unistd.h>
#include <blaze/utils/ProcessInfo.h>

namespace blaze
{
namespace ProcessInfo
{

std::string HostName()
{
    char buf[256];
    if (::gethostname(buf, sizeof(buf)) == 0)
    {
        buf[sizeof(buf) - 1] = '\0';
        return buf;
    }
    else
    {
        return "unknownhost";
    }
}

pid_t pid()
{
    return ::getpid();
}

} // namespace ProcessInfo
} // blaze

