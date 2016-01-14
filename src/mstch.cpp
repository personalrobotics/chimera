#include "chimera/mstch.h"
#include "chimera/util.h"

using namespace clang;

namespace chimera
{
namespace mstch
{

CXXRecord::CXXRecord(
    const CXXRecordDecl *decl,
    const ::chimera::CompiledConfiguration &config)
: ClangWrapper(decl, config)
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
    return chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), QualType(decl_->getTypeForDecl(), 0));
}

::mstch::node CXXRecord::isCopyable()
{
    return chimera::util::isCopyable(decl_);
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