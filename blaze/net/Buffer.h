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


namespace blaze
{
namespace net
{

// a value type Buffer. It's copyable
// implicit copy-ctor, move-ctor, dtor and assignments are OK

class Buffer : public copyable
{
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    explicit Buffer(size_t initial_size = kInitialSize) :
        data_(kCheapPrepend + initial_size),
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
        return data_.size() - write_index_;
    }

    void Swap(Buffer& other)
    {
        data_.swap(other.data_);
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
        assert(size < WritableBytes());
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

    const char* BeginWrite() const
    {
        return Begin() + write_index_;
    }

    char* BeginWrite()
    {
        return Begin() + write_index_;
    }

private:

    char* Begin()
    {
        return data_.data();
    }

    const char* Begin() const
    {
        return data_.data();
    }

    void MakeSpace(size_t size)
    {
        if (WritableBytes() + PrependableBytes() < size + kCheapPrepend)
        {
            // FIXME: move readable bytes to front
            data_.resize(write_index_ + size);
        }
        else
        {
            assert(kCheapPrepend < read_index_);
            size_t readable = ReadableBytes();
            std::copy(Begin() + read_index_, Begin() + write_index_, Begin() + kCheapPrepend);
            read_index_ = kCheapPrepend;
            write_index_ = read_index_ + readable;
            assert(readable == ReadableBytes());
        }
    }

private:
    std::vector<char> data_;
    size_t read_index_;
    size_t write_index_;

    static const char kCRLF[]; // http://www.cnblogs.com/wuyudong/p/c-flexible-array.html
};

} // namespace net
} // namespace blaze

#endif //BLAZE_BUFFER_H
