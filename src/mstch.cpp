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
        {"binding_name", &CXXRecord::bindingName},
        {"uniquish_name", &CXXRecord::uniquishName},
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

::mstch::node CXXRecord::bindingName()
{
    if (const YAML::Node &node = decl_config_["name"])
        return node.as<std::string>();

    return chimera::util::constructBindingName(
        config_.GetContext(), decl_);
}

::mstch::node CXXRecord::uniquishName()
{
    // TODO: implement this properly.
    return name();
}

::mstch::node CXXRecord::mangledName()
{
    if (const YAML::Node &node = decl_config_["mangled_name"])
        return node.as<std::string>();

    return chimera::util::constructMangledName(
        config_.GetContext(), decl_);
}

::mstch::node CXXRecord::constructors()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::methods()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::staticMethods()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::fields()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::staticFields()
{
    return ::mstch::array{std::string{"base"}};
}

Enum::Enum(const clang::EnumDecl *decl,
           const ::chimera::CompiledConfiguration &config)
: ClangWrapper(decl, config)
{
    register_methods(this, {
        {"name", &Enum::name},
        {"type", &Enum::type},
        {"values", &Enum::values}
    });
}

::mstch::node Enum::type()
{
    if (const YAML::Node &node = decl_config_["type"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedDeclTypeAsString(
        config_.GetContext(), decl_);
}

::mstch::node Enum::values()
{
    ::mstch::array constants;
    for (const EnumConstantDecl *constant_decl : decl_->enumerators())
    {
        constants.push_back(
            std::make_shared<EnumConstant>(
                constant_decl, config_));
    }
    return constants;
}

} // namespace mstch
} // namespace chimera
