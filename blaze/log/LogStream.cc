//
// Created by xi on 19-1-8.
//

#include <algorithm>
#include <sstream> // std::stringstream
#include <limits>
#include <blaze/log/LogStream.h>

namespace blaze
{
namespace detail
{

const char digits[] = "9876543210123456789";
const char* zero = digits + 9;
static_assert(sizeof(digits) == 20, "wrong number of digits");

const char digits_hex[] = "0123456789ABCDEF";
static_assert(sizeof(digits_hex) == 17, "wrong number of digits_hex");

//Efficient integer to string conversions, by Matthew Wilson.
template <typename T>
size_t Convert(char buf[], T value)
{
    T i = value;
    char* p = buf;
    do
    {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);
    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

// https://blog.csdn.net/cs_zhanyb/article/details/16973379
size_t ConvertHex(char buf[], uintptr_t value)
{
    uintptr_t i = value;
    char* p = buf;
    do
    {
        int lsd = static_cast<int>(i % 16);
        i /= 16;
        *p++ = digits_hex[lsd];
    } while (i != 0);
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

// explicit template instantiation
// https://stackoverflow.com/questions/2351148/explicit-instantiation-when-is-it-used
template class FixedBuffer<kSmallBuffer>;
template class FixedBuffer<kLargeBuffer>;

// gdb support call a function when debugging
template <int SIZE>
const char* FixedBuffer<SIZE>::DebugString()
{
    *curr_ = '\0';
    return curr_;
}

template <int SIZE>
void FixedBuffer<SIZE>::CookieStart()
{}

template <int SIZE>
void FixedBuffer<SIZE>::CookieEnd()
{}

} //namespace detail

void LogStream::StaticCheck()
{
    //NOTE: https://oomake.com/question/121799
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<double>::digits10,
            "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long double>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long>::digits10,
                  "kMaxNumericSize is large enough");
    static_assert(kMaxNumericSize - 10 > std::numeric_limits<long long>::digits10,
                  "kMaxNumericSize is large enough");
}

template <typename T>
void LogStream::FormatInteger(T v)
{
    if (buffer_.Avail() >= kMaxNumericSize)
    {
        size_t len = detail::Convert(buffer_.Current(), v);
        buffer_.Add(len);
    }
}

LogStream& LogStream::operator<<(short x)
{
    *this << static_cast<int>(x);
    return *this;
}

LogStream& LogStream::operator<<(unsigned short x)
{
    *this << static_cast<unsigned int>(x);
    return *this;
}

LogStream& LogStream::operator<<(int x)
{
    FormatInteger(x);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int x)
{
    FormatInteger(x);
    return *this;
}

LogStream& LogStream::operator<<(long x)
{
    FormatInteger(x);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long x)
{
    FormatInteger(x);
    return *this;
}

LogStream& LogStream::operator<<(long long x)
{
    FormatInteger(x);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long x)
{
    FormatInteger(x);
    return *this;
}

LogStream& LogStream::operator<<(const void* p)
{
    //typedef unsigned long int	uintptr_t; header stdint.h in x64 machine
    //which is the memory address in hex
    uintptr_t v = reinterpret_cast<uintptr_t>(p);
    if (buffer_.Avail() > kMaxNumericSize)
    {
        char* buf = buffer_.Current();
        buf[0] = '0';
        buf[1] = 'x';
        size_t len = detail::ConvertHex(buf + 2, v);
        buffer_.Add(len + 2);
    }
    return *this;
}

// FIXME: efficiency ?

LogStream& LogStream::operator<<(std::thread::id id)
{
    std::stringstream ss;
    ss << id;
    buffer_.Append(ss.str().c_str(), ss.str().size());
    return *this;
}

//FIXME: replace this with Grisu3 by Florian Loitsch
LogStream& LogStream::operator<<(double x)
{
    if (buffer_.Avail() > kMaxNumericSize)
    {
        int len = snprintf(buffer_.Current(), kMaxNumericSize, "%.12g", x);
        buffer_.Add(implicit_cast<size_t>(len));
    }
    return *this;
}

template <typename T>
Fmt::Fmt(const char* fmt, T val)
{
    static_assert(std::is_arithmetic_v<T>, "Must be arithmetic type");
    size_ = snprintf(buf_, sizeof(buf_), fmt, val);
    assert(implicit_cast<size_t>(size_) < sizeof(buf_));
}

// explicit template instantiations
template Fmt::Fmt(const char *fmt, char val);
template Fmt::Fmt(const char *fmt, short val);
template Fmt::Fmt(const char *fmt, unsigned short val);
template Fmt::Fmt(const char *fmt, int val);
template Fmt::Fmt(const char *fmt, unsigned int val);
template Fmt::Fmt(const char *fmt, long val);
template Fmt::Fmt(const char *fmt, unsigned long val);
template Fmt::Fmt(const char *fmt, long long val);
template Fmt::Fmt(const char *fmt, unsigned long long val);
template Fmt::Fmt(const char *fmt, float val);
template Fmt::Fmt(const char *fmt, double val);


} //namespace blaze
