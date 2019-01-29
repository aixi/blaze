//
// Created by xi on 19-1-21.
//

#include <execinfo.h>
#include <stdlib.h>

#include <blaze/utils/Exception.h>

namespace blaze
{

Exception::Exception(const char* what) :
    message_(what)
{
    FillStackTrace();
}

Exception::Exception(const std::string& msg) :
    message_(msg)
{
    FillStackTrace();
}

Exception::~Exception() noexcept = default;

const char* Exception::what() const noexcept
{
    return message_.c_str();
}

const char* Exception::StackTrace() const noexcept
{
    return stack_.c_str();
}

void Exception::FillStackTrace()
{
    const int len = 200;
    void* buffer[len];
    int nptrs = ::backtrace(buffer, len);
    char** strings = ::backtrace_symbols(buffer, nptrs);
    if (strings)
    {
        for (int i = 0; i < nptrs; ++i)
        {
            // TODO: demangle function name with abi::__cxa_demangle
            stack_.append(strings[i]);
            stack_.push_back('\n');
        }
        free(strings);
    }
}

}
