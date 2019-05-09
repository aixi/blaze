//
// Created by xi on 19-5-9.
//

#ifndef BLAZE_ZLIBSTREAM_H
#define BLAZE_ZLIBSTREAM_H

#include <blaze/utils/Types.h>
#include <blaze/net/Buffer.h>

#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <zlib.h>

namespace blaze
{
namespace net
{

// input is zlib compressed data
// output uncompressed data
// TODO: implement this class
class ZlibInputStream
{
public:

    explicit ZlibInputStream(Buffer* output) :
        output_(output),
        zerror_(Z_OK)
    {
        bzero(&zstream_, sizeof(zstream_));
        zerror_ = inflateInit(&zstream_);
    }

    ~ZlibInputStream()
    {
        Finish();
    }

    bool Write(std::string_view buf);
    bool Write(Buffer* input);
    bool Finish();

    DISABLE_COPY_AND_ASSIGN(ZlibInputStream);
private:
    int Decompress(int flush);
private:
    Buffer* output_;
    z_stream zstream_;
    int zerror_;
};

// input is uncompressed data
// output zlib compressed data

class ZlibOutputStream
{
public:

    explicit ZlibOutputStream(Buffer* output) :
        output_(output),
        zerror_(Z_OK),
        buffer_size_(1024)
    {
        bzero(&zstream_, sizeof(zstream_));
        zerror_ = deflateInit(&zstream_, Z_DEFAULT_COMPRESSION);
    }

    ~ZlibOutputStream()
    {
        Finish();
    }

    // Return last error message or nullptr if no error

    const char* ZlibErrorMessage() const
    {
        return zstream_.msg;
    }

    int ZlibErrorCode() const
    {
        return zerror_;
    }

    int64_t InputBytes() const
    {
        return zstream_.total_in;
    }

    int64_t OutputBytes() const
    {
        return zstream_.total_out;
    }

    int InternalOutputBufferSize() const
    {
        return buffer_size_;
    }

    bool Write(std::string_view buf);

    // Compress input as much as possible, not guarantee consuming all data.
    bool Write(Buffer* input);

    bool Finish();

    DISABLE_COPY_AND_ASSIGN(ZlibOutputStream);
private:
    int Compress(int flush);

private:
    Buffer* output_;
    z_stream zstream_;
    int zerror_;
    int buffer_size_;
};

} // namespace net
} // namespace blaze

#endif //BLAZE_ZLIBSTREAM_H
