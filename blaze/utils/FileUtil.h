//
// Created by xi on 19-1-9.
//

#ifndef BLAZE_FILEUTIL_H
#define BLAZE_FILEUTIL_H

#include <sys/types.h>  //off_t

#include <stdio.h>

#include <string>

#include <blaze/utils/noncopyable.h>

namespace blaze
{
namespace FileUtil
{

static const int kBufferSize = 1024 * 64;

//file size < 64KB
class ReadSmallFile : public noncopyable
{
public:
    explicit ReadSmallFile(const std::string& filename);

    ~ReadSmallFile();

    //return errno
    template<typename String>
    int ReadToString(int max_size, String *content,
                                         int64_t *file_size,
                                         int64_t *modify_time,
                                         int64_t *create_time);

    //Read at most kBufferSize byte into buf_
    //return errno
    int ReadToBuffer(int *size);

    const char *buffer() const
    {
        return buf_;
    }


private:
    int fd_;
    int err_;
    char buf_[kBufferSize];
};

template <typename String>
int ReadFile(const std::string& filename, int max_size,
                                          String* content,
                                          int64_t* file_size = nullptr,
                                          int64_t* modify_time = nullptr,
                                          int64_t* create_time = nullptr)
{
    ReadSmallFile file(filename);
    return file.ReadToString(max_size, content, file_size, modify_time, create_time);
}

//NOT thread safe
class AppendFile : public noncopyable
{
public:
    explicit AppendFile(const std::string& filename);

    ~AppendFile();

    void Append(const char* logline, size_t len);

    void Flush();

    off_t WrittenBytes() const
    {
        return written_bytes_;
    }

private:

    size_t Write(const char* logline, size_t len);

private:
    FILE* fp_;
    off_t written_bytes_;
    char buffer_[kBufferSize];
};

} //namespace file_util
} //namespace blaze

#endif //BLAZE_FILEUTIL_H
