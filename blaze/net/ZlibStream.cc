//
// Created by xi on 19-5-9.
//
#include <assert.h>
#include <blaze/net/ZlibStream.h>

namespace blaze
{
namespace net
{

bool ZlibOutputStream::Write(std::string_view buf)
{
    if (zerror_ != Z_OK)
    {
        return false;
    }
    assert(zstream_.next_in == nullptr && zstream_.avail_in == 0);
    void* in = const_cast<char*>(buf.data());
    zstream_.next_in = static_cast<Bytef*>(in);
    zstream_.avail_in = static_cast<unsigned int>(buf.size());
    while (zstream_.avail_in > 0 && zerror_ == Z_OK)
    {
        zerror_ = Compress(Z_NO_FLUSH);
    }
    if (zstream_.avail_in == 0)
    {
        assert(static_cast<const void*>(zstream_.next_in) == buf.end());
        zstream_.next_in = nullptr;
    }
    return zerror_ == Z_OK;
}

bool ZlibOutputStream::Write(Buffer* input)
{
    if (zerror_ != Z_OK)
    {
        return false;
    }
    void* in = const_cast<char*>(input->Peek());
    zstream_.next_in = static_cast<Bytef*>(in);
    zstream_.avail_in = static_cast<unsigned int>(input->ReadableBytes());
    if (zstream_.avail_in > 0 && zerror_ == Z_OK)
    {
        zerror_ = Compress(Z_NO_FLUSH);
    }
    input->Retrieve(input->ReadableBytes() - zstream_.avail_in);
    return zerror_ == Z_OK;
}

bool ZlibOutputStream::Finish()
{
    if (zerror_ != Z_OK)
    {
        return false;
    }
    while (zerror_ == Z_OK)
    {
        zerror_ = Compress(Z_FINISH);
    }
    zerror_ = deflateEnd(&zstream_);
    bool ok = zerror_ == Z_OK;
    zerror_ = Z_STREAM_END;
    return ok;
}

int ZlibOutputStream::Compress(int flush)
{
    output_->EnsureWritableSpace(buffer_size_);
    zstream_.next_out = reinterpret_cast<Bytef*>(output_->BeginWrite());
    zstream_.avail_out = static_cast<int>(output_->WritableBytes());
    int error = deflate(&zstream_, flush);
    output_->HasWritten(output_->WritableBytes() - zstream_.avail_out);
    if (output_->WritableBytes() == 0 && buffer_size_ < 65535)
    {
        buffer_size_ *= 2;
    }
    return error;
}

} // namespace net
} // namespace blaze
