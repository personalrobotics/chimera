#include "chimera/mstch.h"
#include "chimera/util.h"

using namespace clang;

namespace chimera
{
namespace mstch
{

CXXRecord::CXXRecord(
    const ::chimera::CompiledConfiguration &config,
    const CXXRecordDecl *decl)
: ClangWrapper(config, decl)
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
            std::make_shared<CXXRecord>(config_, base_decl));
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

Enum::Enum(const ::chimera::CompiledConfiguration &config,
           const clang::EnumDecl *decl)
: ClangWrapper(config, decl)
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
                config_, constant_decl));
    }
    return constants;
}

Field::Field(const ::chimera::CompiledConfiguration &config,
             const FieldDecl *decl,
             const CXXRecordDecl *class_decl)
: ClangWrapper(config, decl)
, class_decl_(class_decl)
{
    register_methods(this, {
        {"return_value_policy", &Field::returnValuePolicy}
    });
}

::mstch::node Field::returnValuePolicy()
{
    return ::mstch::array{std::string{"base"}};
}

Function::Function(const ::chimera::CompiledConfiguration &config,
                   const clang::FunctionDecl *decl,
                   const CXXRecordDecl *class_decl)
: ClangWrapper(config, decl)
, class_decl_(class_decl)
{
    register_methods(this, {
        {"type", &Function::type},
        {"params", &Function::params},
        {"return_value_policy", &Function::returnValuePolicy}
    });
}

::mstch::node Function::type()
{
    return std::string{"base"};
}

::mstch::node Function::params()
{
    return ::mstch::array{std::string{"base"}};
}

::mstch::node Function::returnValuePolicy()
{
    return std::string{"base"};
}

Var::Var(const ::chimera::CompiledConfiguration &config,
         const clang::VarDecl *decl)
: ClangWrapper(config, decl)
{
    register_methods(this, {
    });
}

} // namespace mstch
} // namespace chimera
