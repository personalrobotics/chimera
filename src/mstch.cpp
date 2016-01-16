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
    // TODO: Implement this method.
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::methods()
{
    // TODO: Implement this method.
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::staticMethods()
{
    // TODO: Implement this method.
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::fields()
{
    // TODO: Implement this method.
    return ::mstch::array{std::string{"base"}};
}

::mstch::node CXXRecord::staticFields()
{
    // TODO: Implement this method.
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
        {"is_assignable", &Field::isAssignable},
        {"is_copyable", &Field::isCopyable},
        {"return_value_policy", &Field::returnValuePolicy}
    });
}

::mstch::node Field::qualifiedName()
{
    if (const YAML::Node &node = decl_config_["qualified_name"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedDeclTypeAsString(
        config_.GetContext(), class_decl_) + "::" + decl_->getNameAsString();
}

::mstch::node Field::isAssignable()
{
    if (const YAML::Node &node = decl_config_["is_assignable"])
        return node.as<std::string>();

    return chimera::util::isAssignable(
        config_.GetContext(), decl_->getType());
}

::mstch::node Field::isCopyable()
{
    if (const YAML::Node &node = decl_config_["is_copyable"])
        return node.as<std::string>();

    return chimera::util::isCopyable(
        config_.GetContext(), decl_->getType());
}

::mstch::node Field::returnValuePolicy()
{
    if (const YAML::Node &node = decl_config_["return_value_policy"])
        return node.as<std::string>();

    return std::string{""};
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
    if (const YAML::Node &node = decl_config_["type"])
        return node.as<std::string>();

    QualType pointer_type;
    if (class_decl_ && !cast<CXXMethodDecl>(decl_)->isStatic())
    {
        pointer_type = config_.GetContext().getMemberPointerType(
            decl_->getType(), class_decl_->getTypeForDecl());
    }
    else
    {
        pointer_type = config_.GetContext().getPointerType(decl_->getType());
    }

    return chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), pointer_type);
}

::mstch::node Function::params()
{
    // TODO: Implement this method.
    return ::mstch::array{std::string{"base"}};
}

::mstch::node Function::returnValuePolicy()
{
    if (const YAML::Node &node = decl_config_["return_value_policy"])
        return node.as<std::string>();

    return std::string{""};
}

::mstch::node Function::qualifiedName()
{
    if (const YAML::Node &node = decl_config_["qualified_name"])
        return node.as<std::string>();

    if (!class_decl_)
        return decl_->getQualifiedNameAsString();

    return chimera::util::getFullyQualifiedDeclTypeAsString(
        config_.GetContext(), class_decl_) + "::" + decl_->getNameAsString();
}

Parameter::Parameter(const ::chimera::CompiledConfiguration &config,
                     const clang::ParmVarDecl *decl,
                     const CXXRecordDecl *class_decl)
: ClangWrapper(config, decl)
, class_decl_(class_decl)
{
    register_methods(this, {
        {"type", &Parameter::type},
        {"value", &Parameter::value}
    });
}

::mstch::node Parameter::type()
{
    if (const YAML::Node &node = decl_config_["type"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), decl_->getType());
}

::mstch::node Parameter::value()
{
    // TODO: Implement this method.
    return std::string{""};
}

Variable::Variable(const ::chimera::CompiledConfiguration &config,
                   const clang::VarDecl *decl)
: ClangWrapper(config, decl)
{
    register_methods(this, {
        // TODO: fill these methods in.
    });
}

} // namespace mstch
} // namespace chimera
