#include "chimera/stream.h"

#include <llvm/Support/raw_ostream.h>

chimera::Stream::Stream(StreamType *stream,
                        const std::string &mangledName,
                        const std::vector<std::string> &includeNames,
                        const std::string &headerSnippet,
                        const std::string &postincludeSnippet,
                        const std::string &footerSnippet)
: stream_(stream)
, footer_snippet_(footerSnippet)
{
    if (!headerSnippet.empty())
        *stream_ << headerSnippet << "\n\n";

    *stream_ << "#include <boost/python.hpp>\n";

    for (const auto &includeName : includeNames)
        *stream_ << "#include <" << includeName << ">\n";
    *stream_ << "\n";

    if (!postincludeSnippet.empty())
        *stream_ << postincludeSnippet << "\n\n";

    *stream << "void " << mangledName << "()\n"
               "{\n";
}

chimera::Stream::~Stream()
{
    *stream_ << "}\n";

    if (!footer_snippet_.empty())
        *stream_ << "\n" << footer_snippet_ << "\n";
}
