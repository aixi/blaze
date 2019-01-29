//
// Created by xi on 19-1-18.
//

#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <blaze/utils/Types.h>
#include <blaze/log/Logging.h>

using namespace blaze;

thread_local char t_errnobuf[512];

thread_local char t_time[32];

thread_local time_t t_last_seconds;

const char* strerror_tl(int saved_errno)
{
    return strerror_r(saved_errno, t_errnobuf, sizeof(t_errnobuf));
}

Logger::LogLevel InitLogLevel()
{
    return Logger::LogLevel::kInfo;
}

Logger::LogLevel g_logLevel = InitLogLevel();

void DefaultOutput(const char* msg, int len)
{
    size_t n = fwrite(msg, 1, len, stdout);
    // FIXME: check n
    UnusedVariable(n);
}

void DefaultFlush()
{
    fflush(stdout);
}

const char* LogLevelNames[ToUType(Logger::LogLevel::NUM_LOG_LEVELS)] = {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL "
};

// a helper class to know string length at compile time

class T
{
public:

    T(const char* str, unsigned len) :
        str_(str),
        len_(len)
    {
        assert(strlen(str) == len);
    }

    const char* str_;
    const unsigned len_;
};

inline LogStream& operator<<(LogStream& s, T v)
{
    s.Append(v.str_, v.len_);
    return s;
}

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v)
{
    s.Append(v.data_, v.size_);
    return s;
}

Logger::OutputFunc g_output = DefaultOutput;
Logger::FlushFunc g_flush = DefaultFlush;

Logger::Impl::Impl(LogLevel level, int old_errno, const Logger::SourceFile &file, int line) :
    time_(Timestamp::Now()),
    stream_(),
    level_(level),
    line_(line),
    basename_(file)
{
    FormatTime();
    stream_ << T(LogLevelNames[ToUType(level_)], 6);
    if (old_errno != 0)
    {
        stream_ << strerror_tl(old_errno) << " (errno=" << old_errno << ") ";
    }
}

void Logger::Impl::FormatTime()
{
    int64_t microseconds_since_epoch = time_.MicrosecondsSinceEpoch();
    time_t seconds = static_cast<time_t>(microseconds_since_epoch / Timestamp::kMicroSecondsPerSecond);
    //int microseconds = static_cast<int>(microseconds_since_epoch % Timestamp::kMicroSecondsPerSecond);
    if (seconds != t_last_seconds)
    {
        t_last_seconds = seconds;
        struct tm tm_time;
        ::localtime_r(&seconds, &tm_time);
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d",
                           tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                           tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 17);
        UnusedVariable(len);
    }
    stream_ << T(t_time, 17) << ' ';
}

void Logger::Impl::Finish()
{
    stream_ << " - " << basename_ << ':' << line_ << '\n';
}

Logger::Logger(Logger::SourceFile file, int line) :
    impl_(LogLevel::kInfo, 0, file, line)
{}

Logger::Logger(Logger::SourceFile file, int line, LogLevel level) :
    impl_(level, 0, file, line)
{}

Logger::Logger(Logger::SourceFile file, int line, LogLevel level, const char *func) :
    impl_(level, 0, file, line)
{
    impl_.stream_ << func << ' ';
}

Logger::Logger(Logger::SourceFile file, int line, bool to_abort) :
    impl_(to_abort ? LogLevel::kFatal : LogLevel::kError, 0, file, line)
{}

Logger::~Logger()
{
    impl_.Finish();
    const LogStream::Buffer &buf(Stream().buffer());
    g_output(buf.Data(), buf.Size());
    if (impl_.level_ == LogLevel::kFatal)
    {
        g_flush();
        abort();
    }
}

void Logger::SetLogLevel(LogLevel level)
{
    g_logLevel = level;
}

void Logger::SetOutput(OutputFunc out)
{
    g_output = out;
}

void Logger::SetFlush(Logger::FlushFunc flush)
{
    g_flush = flush;
}








