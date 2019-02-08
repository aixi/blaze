//
// Created by xi on 19-1-18.
//

#ifndef BLAZE_LOGGING_H
#define BLAZE_LOGGING_H

#include <blaze/log/LogStream.h>
#include <blaze/utils/Timestamp.h>

namespace blaze
{

const char* strerror_tl(int saved_errno);

void DefaultOutput(const char* msg, int len);

// NOTE: enum class can not implicitly cast to integer type

template<typename E>
constexpr std::underlying_type_t<E> ToUType(E enumerator) noexcept
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

class Logger
{
public:

    enum class LogLevel
    {
        kTrace,
        kDebug,
        kInfo,
        kWarn,
        kError,
        kFatal,
        NUM_LOG_LEVELS
    };

    // compile time calculation of base of source file

    class SourceFile
    {
    public:
        template<int SIZE>
        inline SourceFile(const char(& arr)[SIZE]) :
            data_(arr),
            size_(SIZE - 1)
        {
            const char* slash = strchr(data_, '/'); // gcc builtin function
            if (slash)
            {
                data_ = slash + 1;
                size_ -= static_cast<int>(data_ - arr);
            }
        }

        explicit SourceFile(const char* filename) :
            data_(filename)
        {
            const char* slash = strchr(data_, '/'); // gcc builtin function
            if (slash)
            {
                data_ = slash + 1;
                size_ = static_cast<int>(strlen(data_));
            }
        }

        const char* data_;
        int size_;
    };

    Logger(SourceFile file, int line);

    Logger(SourceFile file, int line, LogLevel level);

    Logger(SourceFile file, int line, LogLevel level, const char* func);

    Logger(SourceFile file, int line, bool to_abort);

    ~Logger();

    blaze::LogStream& Stream()
    {
        return impl_.stream_;
    }

    static LogLevel logLevel();

    static void SetLogLevel(LogLevel level);

    using OutputFunc = void (*)(const char* msg, int len);
    using FlushFunc = void (*)();

    static void SetOutput(OutputFunc out);

    static void SetFlush(FlushFunc flush);

private:

    class Impl
    {
    public:
        using LogLevel = Logger::LogLevel;

        Impl(LogLevel level, int old_errno, const SourceFile& file, int line);

        void FormatTime();

        void Finish();

        blaze::Timestamp time_;
        blaze::LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };

    Impl impl_;

};

extern Logger::LogLevel g_logLevel;

inline Logger::LogLevel Logger::logLevel()
{
    return g_logLevel;
}

#undef LOG_TRACE
#undef LOG_DEBUG
#undef LOG_INFO
#undef LOG_WARN
#undef LOG_ERROR
#undef LOG_FATAL
#undef LOG_SYSERR
#undef LOG_SYSFATAL

#define LOG_TRACE if (blaze::Logger::logLevel() <= blaze::Logger::LogLevel::kTrace) \
    blaze::Logger(__FILE__, __LINE__, blaze::Logger::LogLevel::kTrace, __func__).Stream()

#define LOG_DEBUG if (blaze::Logger::logLevel() <= blaze::Logger::LogLevel::kDebug) \
    blaze::Logger(__FILE__, __LINE__, blaze::Logger::LogLevel::kDebug, __func__).Stream()

#define LOG_INFO if (blaze::Logger::logLevel() <= blaze::Logger::LogLevel::kInfo) \
    blaze::Logger(__FILE__, __LINE__).Stream()

#define LOG_WARN blaze::Logger(__FILE__, __LINE__, blaze::Logger::LogLevel::kWarn).Stream()

#define LOG_ERROR blaze::Logger(__FILE__, __LINE__, blaze::Logger::LogLevel::kError).Stream()

#define LOG_FATAL blaze::Logger(__FILE__, __LINE__, blaze::Logger::LogLevel::kFatal).Stream()

#define LOG_SYSERR blaze::Logger(__FILE__, __LINE__, false).Stream()

#define LOG_SYSFATAL blaze::Logger(__FILE__, __LINE__, true).Stream()

// Taken from glog/log.h

#define CHECK_NOTNULL(val) \
    CheckNotNull(__FILE__, __LINE__, "'" #val "' Must be non NULL", (val))

template<typename T>
T* CheckNotNull(blaze::Logger::SourceFile file, int line, const char* names, T* ptr)
{
    if (!ptr)
    {
        blaze::Logger(file, line, Logger::LogLevel::kFatal).Stream() << names;
    }
    return ptr;
}

} // namespace blaze

#endif //BLAZE_LOGGING_H
