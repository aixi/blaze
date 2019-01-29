//
// Created by xi on 19-1-18.
//

#ifndef BLAZE_LOGFILE_H
#define BLAZE_LOGFILE_H

#include <memory>
#include <mutex>
#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Types.h>

namespace blaze
{
namespace FileUtil
{

class AppendFile;

} // namespace FileUtil

class LogFile : public noncopyable
{
public:

    LogFile(const std::string& basename,
            off_t roll_size,
            bool thread_safe = true,
            int flush_interval = 3,
            int check_every_N = 1024);

    // out of line dtor because of std::unique_ptr

    ~LogFile();

    void Append(const char* logline, int len);

    void Flush();

    bool RollFile();

private:

    void AppendUnlocked(const char* logline, int len);

    static std::string GetLogFileName(const std::string& basename, time_t* now);

private:

    const std::string basename_;
    const off_t roll_size_;
    const int flush_interval_;
    const int check_every_N_;

    int count_;

    std::unique_ptr<std::mutex> mutex_;

    time_t start_of_period_;
    time_t last_roll_;
    time_t last_flush_;
    std::unique_ptr<FileUtil::AppendFile> file_;

    const static int kRollPerSeconds_ = 60 * 60 * 24;
};


} // namespace blaze


#endif //BLAZE_LOGFILE_H
