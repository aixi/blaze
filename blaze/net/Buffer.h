//
// Created by xi on 19-1-29.
//

#ifndef BLAZE_BUFFER_H
#define BLAZE_BUFFER_H

#include <string.h>
#include <assert.h>
#include <stddef.h>
#include <vector>
#include <algorithm>
#include <string_view>
#include <string>
#include <blaze/net/Endian.h>
#include <blaze/utils/copyable.h>
#include <blaze/utils/Types.h>


namespace blaze
{
namespace net
{

// a value type Buffer. It's copyable
// implicit copy-ctor, move-ctor, dtor and assignments are OK

class Buffer : public copyable
{
public:
    static const size_t kCheapPrepend;
    static const size_t kInitialSize;

    explicit Buffer(size_t initial_size = kInitialSize) :
        buffer_(kCheapPrepend + initial_size),
        read_index_(kCheapPrepend),
        write_index_(kCheapPrepend)
    {
        assert(PrependableBytes() == kCheapPrepend);
        assert(ReadableBytes() == 0);
        assert(WritableBytes() == kInitialSize);
    }

    size_t ReadableBytes() const
    {
        return write_index_ - read_index_;
    }

    size_t PrependableBytes() const
    {
        return read_index_;
    }

    size_t WritableBytes() const
    {
        return buffer_.size() - write_index_;
    }

    void Swap(Buffer& other)
    {
        buffer_.swap(other.buffer_);
        std::swap(read_index_, other.read_index_);
        std::swap(write_index_, other.write_index_);
    }

    const char* BeginRead() const
    {
        return Begin() + read_index_;
    }

    const char* FindCRLF() const
    {
        const char* crlf = std::search(BeginRead(), BeginWrite(), kCRLF, kCRLF + 2);
        return crlf == BeginWrite() ? nullptr : crlf;
    }

    const char* FindCRLF(const char* start) const
    {
        assert(BeginRead() <= start);
        assert(start <= BeginWrite());
        const char* crlf = std::search(start, BeginWrite(), kCRLF, kCRLF + 2);
        return crlf == BeginWrite() ? nullptr : crlf;
    }

    // Linux only, in Linux the eol is '\n'

    const char* FindEOL() const
    {
        // in gcc char is unsigned, so max char num is 255
        // in msvc char is signed, so max char num is 127
        const void* eol = memchr(BeginRead(), '\n', ReadableBytes());
        return static_cast<const char*>(eol);
    }

    const char* FindEOL(const char* start) const
    {
        assert(BeginRead() <= start);
        assert(start <= BeginWrite());
        const void* eol = memchr(start, '\n', BeginWrite() - start);
        return static_cast<const char*>(eol);
    }

    void Retrieve(size_t size)
    {
        assert(size <= ReadableBytes());
        if (size < ReadableBytes())
        {
            read_index_ += size;
        }
        else
        {
            RetrieveAll();
        }
    }

    void RetrieveUntil(const char* end)
    {
        assert(BeginRead() <= end);
        assert(end <= BeginWrite());
        Retrieve(end - BeginRead());
    }

    void RetrieveInt64()
    {
        Retrieve(sizeof(int64_t));
    }

    void RetrieveInt32()
    {
        Retrieve(sizeof(int32_t));
    }

    void RetrieveInt16()
    {
        Retrieve(sizeof(int16_t));
    }

    void RetrieveInt8()
    {
        Retrieve(sizeof(int8_t));
    }

    std::string RetrieveAllAsString()
    {
        return RetrieveAsString(ReadableBytes());
    }

    std::string RetrieveAsString(size_t size)
    {
        assert(size <= ReadableBytes());
        std::string result(BeginRead(), size);
        Retrieve(size);
        return result;
    }

    void RetrieveAll()
    {
        read_index_ = kCheapPrepend;
        write_index_ = kCheapPrepend;
    }

    void Append(std::string_view sv)
    {
        Append(sv.data(), sv.size());
    }

    void Append(const char* data, size_t size)
    {
        EnsureWritableBytes(size);
        std::copy(data, data + size, BeginWrite());
        HasWritten(size);
    }

    void Append(const void* data, size_t size)
    {
        Append(static_cast<const char*>(data), size);
    }

    void AppendInt8(int8_t x)
    {
        Append(&x, sizeof(x));
    }

    // following AppendInt functions use network endian

    void AppendInt16(int16_t x)
    {
        int16_t be16 = sockets::HostToNetwork16(x);
        Append(&be16, sizeof(be16));
    }

    void AppendInt32(int32_t x)
    {
        int32_t be32 = sockets::HostToNetwork32(x);
        Append(&be32, sizeof(be32));
    }

    void AppendInt64(int64_t x)
    {
        int64_t be64 = sockets::HostToNetwork64(x);
        Append(&be64, sizeof(be64));
    }

    void HasWritten(size_t size)
    {
        assert(size <= WritableBytes());
        write_index_ += size;
    }

    void Unwrite(size_t size)
    {
        assert(size < ReadableBytes());
        write_index_ -= size;
    }

    void EnsureWritableBytes(size_t size)
    {
        if (size > WritableBytes())
        {
            MakeSpace(size);
        }
        assert(WritableBytes() >= size);
    }

    // from network endian
    // return host endian

    int8_t ReadInt8()
    {
        int8_t result = PeekInt8();
        RetrieveInt8();
        return result;
    }

    int16_t ReadInt16()
    {
        int16_t result = PeekInt16();
        RetrieveInt16();
        return result;
    }

    int32_t ReadInt32()
    {
        int32_t result = PeekInt32();
        RetrieveInt32();
        return result;
    }

    int64_t ReadInt64()
    {
        int64_t result = PeekInt64();
        RetrieveInt64();
        return result;
    }

    int8_t PeekInt8() const
    {
        assert(ReadableBytes() >= sizeof(int8_t));
        int8_t be8 = *BeginRead();
        return be8;
    }

    int16_t PeekInt16() const
    {
        assert(ReadableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, BeginRead(), sizeof(be16));
        return sockets::NetworkToHost16(be16);
    }

    int32_t PeekInt32() const
    {
        assert(ReadableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, BeginRead(), sizeof(be32));
        return sockets::NetworkToHost32(be32);
    }

    int64_t PeekInt64() const
    {
        assert(ReadableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, BeginRead(), sizeof(be64));
        return sockets::NetworkToHost64(be64);
    }

    const char* BeginWrite() const
    {
        return Begin() + write_index_;
    }

    std::string ToString() const
    {
        return std::string(BeginRead(), ReadableBytes());
    }

    char* BeginWrite()
    {
        return Begin() + write_index_;
    }

    void Prepend(const void* data, size_t size)
    {
        assert(size <= PrependableBytes());
        read_index_ -= size;
        const char* d = static_cast<const char*>(data);
        std::copy(d, d + size, Begin() + read_index_);
    }

    void Shrink(size_t reserve)
    {
        // FIXME: use vector::shrink_to_fit()
        Buffer other;
        other.EnsureWritableBytes(ReadableBytes()+reserve);
        other.Append(ToString());
        Swap(other);
    }

    size_t InternalCapacity() const
    {
        return buffer_.capacity();
    }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    ssize_t Readfd(int fd, int* saved_errno);

private:

    char* Begin()
    {
        return buffer_.data();
    }

    const char* Begin() const
    {
        return buffer_.data();
    }

    void MakeSpace(size_t size)
    {
        if (WritableBytes() + PrependableBytes() < size + kCheapPrepend)
        {
            //FIXME: move readable bytes to front
            buffer_.resize(write_index_ + size);
        }
        else
        {
            // move readable bytes to front, make room inside buffer
            Transfer();
        }
    }

    void Transfer()
    {
        assert(read_index_ > kCheapPrepend);
        size_t readable = ReadableBytes();
        std::copy(Begin() + read_index_, Begin() + write_index_, Begin() + kCheapPrepend);
        read_index_ = kCheapPrepend;
        write_index_ = read_index_ + readable;
        assert(readable == ReadableBytes());
    }

private:
    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;

    static const char kCRLF[]; // http://www.cnblogs.com/wuyudong/p/c-flexible-array.html
};

} // namespace net
} // namespace blaze

#endif //BLAZE_BUFFER_H
