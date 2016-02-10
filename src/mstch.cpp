#include "chimera/mstch.h"
#include "chimera/util.h"

using namespace clang;

namespace chimera
{
namespace mstch
{

CXXRecord::CXXRecord(
    const ::chimera::CompiledConfiguration &config,
    const CXXRecordDecl *decl,
    const std::set<const CXXRecordDecl*> *available_decls)
: ClangWrapper(config, decl)
, available_decls_(available_decls)
{
    register_methods(this, {
        {"bases", &CXXRecord::bases},
        {"type", &CXXRecord::type},
        {"is_copyable", &CXXRecord::isCopyable},
        {"binding_name", &CXXRecord::bindingName},
        {"uniquish_name", &CXXRecord::uniquishName},
        {"constructors", &CXXRecord::constructors},
        {"methods", &CXXRecord::methods},
        {"fields", &CXXRecord::fields},
        {"static_fields", &CXXRecord::staticFields}
    });
}

::mstch::node CXXRecord::bases()
{
    // Get all bases of this class.
    std::set<const CXXRecordDecl *> base_decls =
        chimera::util::getBaseClassDecls(decl_);

    // If a list of available decls is provided, only use available base classes.
    if (available_decls_)
    {
        std::set<const CXXRecordDecl *> available_base_decls;
        std::copy_if(
            base_decls.begin(), base_decls.end(),
            std::inserter(available_base_decls, available_base_decls.end()),
            [this](const CXXRecordDecl* base_decl)
            {
                return (available_decls_->find(base_decl->getCanonicalDecl()) 
                            != available_decls_->end());
            }
        );
        base_decls = available_base_decls;
    }

    // Convert each base class to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    ::mstch::array base_templates;
    for(auto base_decl : base_decls)
    {
        base_templates.push_back(
            std::make_shared<CXXRecord>(
                config_, base_decl));
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
    if (const YAML::Node &node = decl_config_["binding_name"])
        return node.as<std::string>();

    return chimera::util::constructBindingName(
        config_.GetContext(), decl_);
}

::mstch::node CXXRecord::uniquishName()
{
    // TODO: implement this properly.
    return bindingName();
}

::mstch::node CXXRecord::constructors()
{
    ::mstch::array constructors;

    // If the class is abstract, do not enumerate any constructors.
    if (decl_->isAbstract())
        return constructors;

    for (CXXMethodDecl *const method_decl : decl_->methods())
    {
        if (method_decl->getAccess() != AS_public)
            continue; // skip protected and private members
        if (method_decl->isDeleted())
            continue;
        if (!isa<CXXConstructorDecl>(method_decl))
            continue;
        if (chimera::util::containsRValueReference(method_decl))
        {
            std::cerr
                << "Warning: Skipped constructor of "
                << decl_->getNameAsString()
                << " because a parameter was an rvalue reference.\n";
            continue;
        }

        const CXXConstructorDecl *constructor_decl =
            cast<CXXConstructorDecl>(method_decl);
        if (constructor_decl->isCopyOrMoveConstructor())
            continue;

        constructors.push_back(
            std::make_shared<Function>(
                config_, method_decl, decl_));
    }
    return constructors;
}

::mstch::node CXXRecord::methods()
{
    ::mstch::array methods;
    for (CXXMethodDecl *const method_decl : decl_->methods())
    {
        if (method_decl->getAccess() != AS_public)
            continue; // skip protected and private members
        if (isa<CXXConversionDecl>(method_decl))
            continue;
        if (isa<CXXDestructorDecl>(method_decl))
            continue;
        if (isa<CXXConstructorDecl>(method_decl))
            continue;
        if (method_decl->isOverloadedOperator())
            continue;
        if (method_decl->isStatic())
            continue;
        if (method_decl->isDeleted())
            continue;
        if (method_decl->getDescribedFunctionTemplate())
            continue;
        // Skip functions that have incomplete argument types. Boost.Python
        // requires RTTI information about all arguments, including references
        // and pointers.
        if (chimera::util::containsIncompleteType(method_decl))
        {
            std::cerr
                << "Warning: Skipped function "
                << method_decl->getQualifiedNameAsString()
                << " because an argument has an incomplete type.\n";
            continue;
        }

        methods.push_back(
            std::make_shared<Function>(
                    config_, method_decl, decl_));
    }
    return methods;
}

::mstch::node CXXRecord::fields()
{
    ::mstch::array fields;
    for (FieldDecl *const field_decl : decl_->fields())
    {
        if (field_decl->getAccess() != AS_public)
            continue; // skip protected and private fields

        if (!chimera::util::isCopyable(
                config_.GetContext(), field_decl->getType()))
            continue;

        fields.push_back(
            std::make_shared<Field>(
                config_, field_decl, decl_));
    }
    return fields;
}

::mstch::node CXXRecord::staticFields()
{
    ::mstch::array static_fields;
    for (Decl *const child_decl : decl_->decls())
    {
        if (!isa<VarDecl>(child_decl))
            continue;

        const VarDecl *static_field_decl = cast<VarDecl>(child_decl);
        if (static_field_decl->getAccess() != AS_public)
            continue;
        else if (!static_field_decl->isStaticDataMember())
            continue;

        static_fields.push_back(
            std::make_shared<Variable>(
                config_, static_field_decl, decl_));
    }
    return static_fields;
}

Enum::Enum(const ::chimera::CompiledConfiguration &config,
           const EnumDecl *decl)
: ClangWrapper(config, decl)
{
    register_methods(this, {
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

::mstch::node Field::qualifiedName()
{
    if (const YAML::Node &node = decl_config_["qualified_name"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedDeclTypeAsString(
        config_.GetContext(), class_decl_) + "::" + decl_->getNameAsString();
}

Function::Function(const ::chimera::CompiledConfiguration &config,
                   const FunctionDecl *decl,
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
    ::mstch::array params;
    for (const ParmVarDecl *param_decl : decl_->params())
    {
        // TODO: implement default value filtering.
        params.push_back(
            std::make_shared<Parameter>(
                config_, param_decl, decl_, class_decl_));
    }
    return params;
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
                     const ParmVarDecl *decl,
                     const FunctionDecl *method_decl,
                     const CXXRecordDecl *class_decl,
                     bool use_default)
: ClangWrapper(config, decl)
, class_decl_(class_decl)
, method_decl_(method_decl)
, use_default_(use_default)
{
    register_methods(this, {
        {"name", &Parameter::name},
        {"type", &Parameter::type},
        {"value", &Parameter::value}
    });
}

::mstch::node Parameter::name()
{
    if (const YAML::Node &node = decl_config_["name"])
        return node.as<std::string>();

    return decl_->getNameAsString();
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
    if (!decl_->hasDefaultArg())
        return "";
    if (decl_->hasUninstantiatedDefaultArg())
        return "";
    if (decl_->hasUnparsedDefaultArg())
        return "";

    const Expr *default_expr = decl_->getDefaultArg();
    assert(default_expr);

    Expr::EvalResult result;
    bool success;

    const Type *param_type = decl_->getType().getTypePtr();
    if (param_type->isReferenceType())
        success = default_expr->EvaluateAsLValue(result, config_.GetContext());
    else
        success = default_expr->EvaluateAsRValue(result, config_.GetContext());

    std::string param_value;
    if (success)
    {
        // Handle special cases for infinite and nan float values.
        // These values resolve to internal compiler definitions, so
        // their default string serialization won't resolve correctly.
        if (result.Val.isFloat() && result.Val.getFloat().isInfinity())
        {
            param_value = result.Val.getFloat().isNegative()
                ? "-INFINITY" : "INFINITY";
        }
        else if (result.Val.isFloat() && result.Val.getFloat().isNaN())
        {
            param_value = "NAN";
        }
        else
        {
            param_value = result.Val.getAsString(
                config_.GetContext(), decl_->getType());
        }
    }
    // TODO: Ask Clang developers why this requires a const-cast?
    else if (const_cast<Expr *>(default_expr)->hasNonTrivialCall(config_.GetContext()))
    {
        // TODO: How do we print the decl with argument + return types?
        const std::string param_name = decl_->getNameAsString();
        std::cerr
          << "Warning: Unable to evaluate non-trivial call in default"
             " value for parameter"
          << " '" << param_name << "' of method"
          << " '" << method_decl_->getQualifiedNameAsString() << "'.\n";
    }
    else
    {
        // TODO: How do we print the decl with argument + return types?
        const std::string param_name = decl_->getNameAsString();
        std::cerr
          << "Warning: Failed to evaluate default value for parameter"
          << " '" << param_name << "' of method"
          << " '" << method_decl_->getQualifiedNameAsString() << "'.\n";
    }

    // If a constant has been overriden, use the override instead of the
    // original value.  Currently, this is just done via string-matching.
    return config_.GetConstant(param_value);
}

Variable::Variable(const ::chimera::CompiledConfiguration &config,
                   const VarDecl *decl,
                   const CXXRecordDecl *class_decl)
: ClangWrapper(config, decl)
, class_decl_(class_decl)
{
    register_methods(this, {
        {"is_assignable", &Variable::isAssignable},
    });
}

::mstch::node Variable::qualifiedName()
{
    if (const YAML::Node &node = decl_config_["qualified_name"])
        return node.as<std::string>();

    if (!class_decl_)
        return decl_->getQualifiedNameAsString();

    return chimera::util::getFullyQualifiedDeclTypeAsString(
        config_.GetContext(), class_decl_) + "::" + decl_->getNameAsString();
}

::mstch::node Variable::isAssignable()
{
    if (const YAML::Node &node = decl_config_["is_assignable"])
        return node.as<std::string>();

    return chimera::util::isAssignable(
        config_.GetContext(), decl_->getType());
}

} // namespace mstch
} // namespace chimera
