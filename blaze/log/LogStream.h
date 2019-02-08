//
// Created by xi on 19-1-8.
//

#ifndef BLAZE_LOGSTREAM_H
#define BLAZE_LOGSTREAM_H

#include <assert.h>
#include <string.h> // memcpy
#include <string>
#include <thread> // std::thread::id
#include <blaze/utils/noncopyable.h>
#include <blaze/utils/Types.h>


namespace blaze
{
namespace detail
{

const int kSmallBuffer = 4096; //4MB
const int kLargeBuffer = 4096 * 10; //40MB

template <int SIZE>
class FixedBuffer
{
public:

    FixedBuffer() :
        curr_(data_)
    {
        SetCookie(CookieStart);
    }

    ~FixedBuffer()
    {
        SetCookie(CookieEnd);
    }

    void Append(const char* buf, size_t len)
    {
        //FIXME: append partially
        if (implicit_cast<size_t>(Avail()) > len)
        {
            memcpy(curr_, buf, len);
            curr_ += len;
        }
    }

    const char* Data() const
    {
        return data_;
    }

    int Size() const
    {
        return static_cast<int>(curr_ - data_);
    }

    char* Current()
    {
        return curr_;
    }

    int Avail() const
    {
        return static_cast<int>(End() - curr_);
    }

    void Add(size_t len)
    {
        curr_ += len;
    }

    void Reset()
    {
        curr_ = data_;
    }

    void bzero()
    {
        ::bzero(data_, sizeof(data_));
    }

    //used by GDB
    const char* DebugString();

    void SetCookie(void (*cookie)())
    {
        cookie_ = cookie;
    }

    //used in unit tests
    std::string ToString() const
    {
        return std::string(data_, Size()); //RVO
    }

private:

    const char* End() const
    {
        return data_ + sizeof(data_);
    }

    //if program crush, how to save last log line
    //using cookie_ to save in core dump file
    void (*cookie_)();
    //must be out-line function for cookies
    static void CookieStart();
    static void CookieEnd();

private:
    char data_[SIZE];
    char* curr_;

};

} //namespace detail


class LogStream : public noncopyable
{
    using Self = LogStream;

public:
    using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

    Self& operator<<(bool x)
    {
        buffer_.Append(x ? "1" : "0", 1);
        return *this;
    }

    Self& operator<<(short x);
    Self& operator<<(unsigned short x);
    Self& operator<<(int x);
    Self& operator<<(unsigned int x);
    Self& operator<<(long x);
    Self& operator<<(unsigned long x);
    Self& operator<<(long long x);
    Self& operator<<(unsigned long long x);
    Self& operator<<(double x);
    Self& operator<<(std::thread::id id);
    //Self& operator<<(long double x);
    //Self& operator<<(signed char x);
    //Self& operator<<(unsigned char x);
    Self& operator<<(const void* x);

    Self& operator<<(float x)
    {
        *this << static_cast<double>(x);
        return *this;
    }

    Self& operator<<(char x)
    {
        buffer_.Append(&x, 1);
        return *this;
    }

    Self& operator<<(const char* str)
    {
        if (str)
        {
            buffer_.Append(str, strlen(str));
        }
        else
        {
            buffer_.Append("(null)", 6);
        }
        return *this;
    }

    Self& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }


    Self& operator<<(const std::string& str)
    {
        buffer_.Append(str.c_str(), str.size());
        return *this;
    }

    Self& operator<<(const Buffer& buf)
    {
        *this << buf.ToString();
        return *this;
    }

    void Append(const char* data, int len)
    {
        buffer_.Append(data, len);
    }

    const Buffer& buffer() const
    {
        return buffer_;
    }

    void ResetBuffer()
    {
        buffer_.Reset();
    }


private:
    void StaticCheck();

    template <typename T>
    void FormatInteger(T v);

private:
    Buffer buffer_;
    static const int kMaxNumericSize = 32;
};

class Fmt
{
public:

    template <typename T>
    Fmt(const char* fmt, T val);

    const char* Data() const
    {
        return buf_;
    }

    int Size() const
    {
        return size_;
    }

private:
    char buf_[32];
    int size_;
};

inline LogStream& operator<<(LogStream& stream, const Fmt& fmt)
{
    stream.Append(fmt.Data(), fmt.Size());
    return stream;
}

} //namespace blaze

#endif //BLAZE_LOGSTREAM_H
