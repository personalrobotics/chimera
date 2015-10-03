#include "chimera/stream.h"

#include <llvm/Support/raw_ostream.h>

chimera::Stream::Stream(StreamType *stream,
                        const std::string &includeName,
                        const std::string &mangledName)
: stream_(stream)
{
    *stream_ << "#include <" << includeName << ">\n"
                "\n"
                "void " << mangledName << "()\n"
                "{\n";
}

chimera::Stream::~Stream()
{
    *stream_ << "}\n";
}
