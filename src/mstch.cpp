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
        {"type", &CXXRecord::type},
        {"is_copyable", &CXXRecord::isCopyable},
        {"name", &CXXRecord::name},
        {"mangled_name", &CXXRecord::mangledName}
    });
}

::mstch::node CXXRecord::bases()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::type()
{
    if (decl_config_["type"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), QualType(decl_->getTypeForDecl(), 0));
}

::mstch::node CXXRecord::isCopyable()
{
    if (decl_config_["is_copyable"])
        return node.as<bool>();

    return chimera::util::isCopyable(decl_);
}

::mstch::node CXXRecord::name()
{
    if (decl_config_["name"])
        return node.as<std::string>();

    return chimera::util::constructBindingName(
        config_.GetContext(), decl_);
}

::mstch::node chimera::mstch::CXXRecord::uniquishName()
{
    // TODO: implement this properly.
    return name();
}

::mstch::node chimera::mstch::CXXRecord::mangledName()
{
    if (decl_config_["mangled_name"])
        return node.as<std::string>();

    return chimera::util::constructMangledName
        config_.GetContext(), decl_);
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