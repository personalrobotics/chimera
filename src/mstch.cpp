#include "chimera/mstch.h"

namespace chimera
{
namespace mstch
{

CXXRecord::CXXRecord(
    const clang::CXXRecordDecl *decl,
    const ::chimera::CompiledConfiguration &config)
: decl_(decl)
, config_(config)
{
    register_methods(this, {
        {"bases", &CXXRecord::bases},
        {"type", &CXXRecord::type}
    });
}

::mstch::node CXXRecord::bases()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::type()
{
    return std::string{"type"};
}

::mstch::node CXXRecord::isCopyable()
{
    return false;
}

::mstch::node CXXRecord::name()
{
    return std::string{"type"};
}

::mstch::node chimera::mstch::CXXRecord::uniquishName()
{
    return std::string{"uniquishName"};
}

::mstch::node chimera::mstch::CXXRecord::mangledName()
{
    return std::string{"mangledName"};
}

::mstch::node chimera::mstch::CXXRecord::constructors()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node chimera::mstch::CXXRecord::methods()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node chimera::mstch::CXXRecord::staticMethods()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node chimera::mstch::CXXRecord::fields()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node chimera::mstch::CXXRecord::staticFields()
{
    return ::mstch::array{std::string{"base"}};
}

} // namespace mstch
} // namespace chimera