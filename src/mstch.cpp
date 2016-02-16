#include "chimera/configuration.h"
#include "chimera/mstch.h"
#include "chimera/util.h"

#include <vector>

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
    ::mstch::array base_templates;

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
    std::vector<std::shared_ptr<CXXRecord>> base_vector;
    for(const auto *base_decl : base_decls)
    {
        base_vector.push_back(
            std::make_shared<CXXRecord>(
                config_, base_decl));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (base_vector.size() > 0)
        base_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto base_template : base_vector)
        base_templates.push_back(base_template);
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
    ::mstch::array constructor_templates;

    // If the class is abstract, do not enumerate any constructors.
    if (decl_->isAbstract())
        return constructor_templates;

    // Convert each constructor to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Function>> constructor_vector;
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

        constructor_vector.push_back(
            std::make_shared<Function>(
                config_, method_decl, decl_));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (constructor_vector.size() > 0)
        constructor_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto constructor_template : constructor_vector)
        constructor_templates.push_back(constructor_template);
    return constructor_templates;
}

::mstch::node CXXRecord::methods()
{
    ::mstch::array method_templates;

    // Convert each method to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Function>> method_vector;
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

        // Generate the method wrapper (but don't add it just yet).
        auto method = std::make_shared<Function>(config_, method_decl, decl_);

        // Check if a return_value_policy can be generated for this function.
        if (::mstch::render("{{return_value_policy}}", method).empty()
                && chimera::util::needsReturnValuePolicy(
                    method_decl, method_decl->getReturnType().getTypePtr()))
            continue;

        // Now that we know it can be generated, add the method.
        method_vector.push_back(method);
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (method_vector.size() > 0)
        method_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto method_template : method_vector)
        method_templates.push_back(method_template);
    return method_templates;
}

::mstch::node CXXRecord::fields()
{
    ::mstch::array field_templates;

    // Convert each field to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Field>> field_vector;
    for (FieldDecl *const field_decl : decl_->fields())
    {
        if (field_decl->getAccess() != AS_public)
            continue; // skip protected and private fields

        if (!chimera::util::isCopyable(
                config_.GetContext(), field_decl->getType()))
            continue;

        field_vector.push_back(
            std::make_shared<Field>(
                config_, field_decl, decl_));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (field_vector.size() > 0)
        field_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto field_template : field_vector)
        field_templates.push_back(field_template);
    return field_templates;
}

::mstch::node CXXRecord::staticFields()
{
    ::mstch::array static_field_templates;

    // Convert each static field to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Variable>> static_field_vector;
    for (Decl *const child_decl : decl_->decls())
    {
        if (!isa<VarDecl>(child_decl))
            continue;

        const VarDecl *static_field_decl = cast<VarDecl>(child_decl);
        if (static_field_decl->getAccess() != AS_public)
            continue;
        if (!static_field_decl->isStaticDataMember())
            continue;

        // Check if a return_value_policy can be generated for this function.
        if (chimera::util::needsReturnValuePolicy(
                static_field_decl, static_field_decl->getType().getTypePtr()))
            continue;

        static_field_vector.push_back(
            std::make_shared<Variable>(
                config_, static_field_decl, decl_));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (static_field_vector.size() > 0)
        static_field_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto static_field_template : static_field_vector)
        static_field_templates.push_back(static_field_template);
    return static_field_templates;
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
    ::mstch::array constant_templates;

    // Convert each enum constant to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<EnumConstant>> constant_vector;
    for (const EnumConstantDecl *constant_decl : decl_->enumerators())
    {
        constant_vector.push_back(
            std::make_shared<EnumConstant>(
                config_, constant_decl));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (constant_vector.size() > 0)
        constant_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto constant_template : constant_templates)
        constant_templates.push_back(constant_template);
    return constant_templates;
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
    ::mstch::array param_templates;

    // Convert each parameter to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Parameter>> param_vector;
    for (const ParmVarDecl *param_decl : decl_->params())
    {
        // TODO: implement default value filtering.
        param_vector.push_back(
            std::make_shared<Parameter>(
                config_, param_decl, decl_, class_decl_));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (param_vector.size() > 0)
        param_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto param_template : param_templates)
        param_templates.push_back(param_template);
    return param_templates;
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
