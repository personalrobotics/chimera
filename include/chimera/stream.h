#ifndef __CHIMERA_STREAM_H__
#define __CHIMERA_STREAM_H__

#include <string>

// Forward declare LLVM raw_ostream, as per:
// http://llvm.org/docs/CodingStandards.html#use-raw-ostream
namespace llvm
{
class raw_ostream;
}

namespace
{
typedef llvm::raw_ostream StreamType;
}

namespace chimera
{

/**
 * Opaque stream type that forwards to internal file stream.
 */
class Stream
{
public:
    Stream(StreamType *stream,
           const std::string &includeName,
           const std::string &mangledName);
    virtual ~Stream();

    Stream(const Stream&) = delete;
    void operator=(const Stream&) = delete;

    template<typename T> Stream& operator<<(const T& obj);

private:
    StreamType *stream_; // Pointer to actual stream type.
};


template<typename T> Stream& Stream::operator<<(const T& obj)
{
    *stream_ << obj;
    return *this;
}

} // namespace chimera

#endif // __CHIMERA_STREAM_H__
