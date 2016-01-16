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
    // TODO: add filtering for undefined base classes.
    std::set<const CXXRecordDecl *> base_decls =
        chimera::util::getBaseClassDecls(decl_);

    ::mstch::array base_templates;
    for(auto base_decl : base_decls)
    {
        base_templates.push_back(
            std::make_shared<CXXRecord>(base_decl, config_));
    }

    return base_templates;
}

::mstch::node CXXRecord::type()
{
    if (const YAML::Node &node = decl_config_["type"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), QualType(decl_->getTypeForDecl(), 0));
}

::mstch::node CXXRecord::isCopyable()
{
    if (const YAML::Node &node = decl_config_["is_copyable"])
        return node.as<bool>();

    return chimera::util::isCopyable(decl_);
}

::mstch::node CXXRecord::name()
{
    if (const YAML::Node &node = decl_config_["name"])
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
    if (const YAML::Node &node = decl_config_["mangled_name"])
        return node.as<std::string>();

    return chimera::util::constructMangledName(
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