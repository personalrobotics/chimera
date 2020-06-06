#include "chimera/mstch.h"
#include "chimera/configuration.h"
#include "chimera/util.h"
#include "cling_utils_AST.h"

#include <iostream>
#include <sstream>
#include <unordered_set>
#include <vector>
#include <boost/variant/get.hpp>

using namespace clang;

namespace chimera
{
namespace mstch
{

::mstch::node generateNamespaceScope(
    const ::chimera::CompiledConfiguration &config,
    const NestedNameSpecifier *nns)
{
    ::mstch::array scope_templates;
    while (nns)
    {
        switch (nns->getKind())
        {
            case NestedNameSpecifier::Namespace:
            {
                NamespaceDecl *parent_decl
                    = nns->getAsNamespace()->getCanonicalDecl();
                auto ns = std::make_shared<Namespace>(config, parent_decl);
                if (!ns->nameAsString().empty())
                    scope_templates.push_back(ns);
                break;
            }
            case NestedNameSpecifier::TypeSpec:
                break;
            case NestedNameSpecifier::Global:
            case NestedNameSpecifier::Identifier:
            case NestedNameSpecifier::NamespaceAlias:
            case NestedNameSpecifier::Super:
            case NestedNameSpecifier::TypeSpecWithTemplate:
                throw std::runtime_error(
                    "Unsupported type of NestedNameSpecifier.");
            default:
                throw std::runtime_error(
                    "Unknown type of NestedNameSpecifier.");
        }

        nns = nns->getPrefix();
    }

    std::reverse(scope_templates.begin(), scope_templates.end());
    return scope_templates;
}

::mstch::node generateNamespaceScope(
    const ::chimera::CompiledConfiguration &config,
    const clang::DeclContext *decl_context)
{
    if (!decl_context)
        throw std::runtime_error("Decl was not enclosed by a context.");

    auto namespace_decl = llvm::dyn_cast_or_null<NamespaceDecl>(decl_context);
    if (!namespace_decl)
    {
        std::stringstream ss;
        ss << "Decl was not enclosed by a namespace; got '"
           << decl_context->getDeclKindName() << "' instead.";
        throw std::runtime_error(ss.str());
    }

    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config.GetContext(), namespace_decl->getCanonicalDecl());

    return generateNamespaceScope(config, nns);
}

::mstch::node generateClassScope(const ::chimera::CompiledConfiguration &config,
                                 const NestedNameSpecifier *nns)
{
    ::mstch::array scope_templates;
    while (nns)
    {
        switch (nns->getKind())
        {
            case NestedNameSpecifier::TypeSpec:
            {
                CXXRecordDecl *parent_decl
                    = nns->getAsType()->getAsCXXRecordDecl();
                if (!parent_decl)
                    throw std::runtime_error(
                        "TypeSpec was not a CXXRecordDecl.");
                auto cxx_record
                    = std::make_shared<CXXRecord>(config, parent_decl);
                if (!cxx_record->nameAsString().empty())
                    scope_templates.push_back(cxx_record);
                break;
            }
            case NestedNameSpecifier::Namespace:
                break;
            case NestedNameSpecifier::Global:
            case NestedNameSpecifier::Identifier:
            case NestedNameSpecifier::NamespaceAlias:
            case NestedNameSpecifier::Super:
            case NestedNameSpecifier::TypeSpecWithTemplate:
                throw std::runtime_error(
                    "Unsupported type of NestedNameSpecifier.");
            default:
                throw std::runtime_error(
                    "Unknown type of NestedNameSpecifier.");
        }

        nns = nns->getPrefix();
    }

    std::reverse(scope_templates.begin(), scope_templates.end());
    return scope_templates;
}

::mstch::node generateClassScope(const ::chimera::CompiledConfiguration &config,
                                 const clang::DeclContext *decl_context)
{
    if (!decl_context)
        throw std::runtime_error("Decl was not enclosed by a context.");

    auto namespace_decl = llvm::dyn_cast_or_null<NamespaceDecl>(decl_context);
    if (!namespace_decl)
    {
        std::stringstream ss;
        ss << "Decl was not enclosed by a namespace; got '"
           << decl_context->getDeclKindName() << "' instead.";
        throw std::runtime_error(ss.str());
    }

    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config.GetContext(), namespace_decl->getCanonicalDecl());

    return generateClassScope(config, nns);
}

::mstch::node generateScope(const ::chimera::CompiledConfiguration &config,
                            const NestedNameSpecifier *nns)
{
    ::mstch::array scope_templates;
    while (nns)
    {
        switch (nns->getKind())
        {
            case NestedNameSpecifier::Namespace:
            {
                NamespaceDecl *parent_decl
                    = nns->getAsNamespace()->getCanonicalDecl();
                auto ns = std::make_shared<Namespace>(config, parent_decl);
                if (!ns->nameAsString().empty())
                    scope_templates.push_back(ns);
                break;
            }
            case NestedNameSpecifier::TypeSpec:
            {
                CXXRecordDecl *parent_decl
                    = nns->getAsType()->getAsCXXRecordDecl();
                if (!parent_decl)
                    throw std::runtime_error(
                        "TypeSpec was not a CXXRecordDecl.");
                auto cxx_record
                    = std::make_shared<CXXRecord>(config, parent_decl);
                if (!cxx_record->nameAsString().empty())
                    scope_templates.push_back(cxx_record);
                break;
            }
            case NestedNameSpecifier::Global:
            case NestedNameSpecifier::Identifier:
            case NestedNameSpecifier::NamespaceAlias:
            case NestedNameSpecifier::Super:
            case NestedNameSpecifier::TypeSpecWithTemplate:
                throw std::runtime_error(
                    "Unsupported type of NestedNameSpecifier.");
            default:
                throw std::runtime_error(
                    "Unknown type of NestedNameSpecifier.");
        }

        nns = nns->getPrefix();
    }

    std::reverse(scope_templates.begin(), scope_templates.end());
    return scope_templates;
}

::mstch::node generateScope(const ::chimera::CompiledConfiguration &config,
                            const clang::DeclContext *decl_context)
{
    if (!decl_context)
        throw std::runtime_error("Decl was not enclosed by a context.");

    auto namespace_decl = llvm::dyn_cast_or_null<NamespaceDecl>(decl_context);
    if (!namespace_decl)
    {
        std::stringstream ss;
        ss << "Decl was not enclosed by a namespace; got '"
           << decl_context->getDeclKindName() << "' instead.";
        throw std::runtime_error(ss.str());
    }

    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config.GetContext(), namespace_decl->getCanonicalDecl());

    return generateScope(config, nns);
}

CXXRecord::CXXRecord(const ::chimera::CompiledConfiguration &config,
                     const CXXRecordDecl *decl,
                     const std::set<const CXXRecordDecl *> *available_decls)
  : ClangWrapper(config, decl), available_decls_(available_decls)
{
    register_methods(
        this,
        {
            {"bases", &CXXRecord::bases},
            {"bases?", &CXXRecord::isNonFalse<CXXRecord, &CXXRecord::bases>},
            {"type", &CXXRecord::type},
            {"is_copyable", &CXXRecord::isCopyable},
            {"constructors", &CXXRecord::constructors},
            {"constructors?",
             &CXXRecord::isNonFalse<CXXRecord, &CXXRecord::constructors>},
            {"methods", &CXXRecord::methods},
            {"methods?",
             &CXXRecord::isNonFalse<CXXRecord, &CXXRecord::methods>},
            {"static_methods", &CXXRecord::staticMethods},
            {"static_methods?",
             &CXXRecord::isNonFalse<CXXRecord, &CXXRecord::methods>},
            {"visible_methods", &CXXRecord::visibleMethods},
            {"visible_methods?",
             &CXXRecord::isNonFalse<CXXRecord, &CXXRecord::visibleMethods>},
            {"fields", &CXXRecord::fields},
            {"fields?", &CXXRecord::isNonFalse<CXXRecord, &CXXRecord::fields>},
            {"static_fields", &CXXRecord::staticFields},
            {"static_fields?",
             &CXXRecord::isNonFalse<CXXRecord, &CXXRecord::staticFields>},
        });
}

::mstch::node CXXRecord::bases()
{
    if (const YAML::Node node = decl_config_["bases"])
        return chimera::util::wrapYAMLNode(node);

    // Get all bases of this class.
    std::set<const CXXRecordDecl *> base_decls
        = chimera::util::getBaseClassDecls(decl_);

    // If a list of available decls is provided, only use available base
    // classes.
    if (available_decls_)
    {
        std::set<const CXXRecordDecl *> available_base_decls;
        std::copy_if(
            base_decls.begin(), base_decls.end(),
            std::inserter(available_base_decls, available_base_decls.end()),
            [this](const CXXRecordDecl *base_decl) {
                return (available_decls_->find(base_decl->getCanonicalDecl())
                        != available_decls_->end());
            });
        base_decls = available_base_decls;
    }

    // Convert each base class to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<CXXRecord>> base_vector;
    for (const auto *base_decl : base_decls)
    {
        base_vector.push_back(std::make_shared<CXXRecord>(config_, base_decl));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (base_vector.size() > 0)
        base_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    ::mstch::array base_templates;
    for (auto base_template : base_vector)
        base_templates.push_back(base_template);
    return base_templates;
}

::mstch::node CXXRecord::scope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateScope(config_, nns->getPrefix());
}

std::string CXXRecord::typeAsString()
{
    if (const YAML::Node node = decl_config_["type"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), QualType(decl_->getTypeForDecl(), 0));
}

::mstch::node CXXRecord::type()
{
    return typeAsString();
}

::mstch::node CXXRecord::namespaceScope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateNamespaceScope(config_, nns->getPrefix());
}

::mstch::node CXXRecord::classScope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateClassScope(config_, nns->getPrefix());
}

::mstch::node CXXRecord::isCopyable()
{
    if (const YAML::Node node = decl_config_["is_copyable"])
        return node.as<bool>();

    return chimera::util::isCopyable(decl_);
}

::std::string CXXRecord::nameAsString()
{
    if (const YAML::Node node = decl_config_["name"])
        return node.as<std::string>();

    return chimera::util::constructBindingName(decl_);
}

::mstch::node CXXRecord::qualifiedName()
{
    if (const YAML::Node node = decl_config_["qualified_name"])
        return node.as<std::string>();

    // In the special case of CXXRecords, the fully-qualified name of the
    // class is pretty much always identical to the type declaration. Since
    // the clang typename resolution works better than the qualified name
    // resolution, we simply use it again here.
    return type();
}

::mstch::node CXXRecord::constructors()
{
    ::mstch::array constructor_templates;

    // If the class is abstract, do not enumerate any constructors.
    if (decl_->isAbstract())
        return constructor_templates;

    // Convert each constructor to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Method>> constructor_vector;
    for (CXXMethodDecl *const method_decl : decl_->methods())
    {
        if (config_.IsSuppressed(method_decl))
            continue;
        if (method_decl->getAccess() != AS_public)
            continue; // skip protected and private members
        if (chimera::util::hasNonPublicParam(method_decl))
            continue;
        if (method_decl->isDeleted())
            continue;
        if (!isa<CXXConstructorDecl>(method_decl))
            continue;
        if (chimera::util::containsRValueReference(method_decl))
            continue;

        // Skip functions that have incomplete argument types. Boost.Python
        // requires RTTI information about all arguments, including references
        // and pointers.
        if (chimera::util::containsIncompleteType(
                config_.GetCompilerInstance()->getSema(), method_decl))
            continue;

        // Skip functions that have non-copyable argument types.
        // Using these functions requires std::move()-ing their arguments, which
        // we generally cannot do.
        if (chimera::util::containsNonCopyableType(method_decl))
            continue;

        const CXXConstructorDecl *constructor_decl
            = cast<CXXConstructorDecl>(method_decl);
        if (constructor_decl->isCopyOrMoveConstructor())
            continue;

        constructor_vector.push_back(
            std::make_shared<Method>(config_, method_decl, decl_));
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
    const ::mstch::array method_templates
        = boost::get<::mstch::array>(methodsInternal());

    // Add all non-static method templates to this list.
    ::mstch::array non_static_methods;
    for (const ::mstch::node &method_node : method_templates)
    {
        const auto method
            = boost::get<std::shared_ptr<::mstch::object>>(method_node);
        const bool is_static = boost::get<bool>(method->at("is_static"));
        if (!is_static)
            non_static_methods.push_back(method);
    }

    return non_static_methods;
}

::mstch::node CXXRecord::staticMethods()
{
    const ::mstch::array method_templates
        = boost::get<::mstch::array>(methodsInternal());

    // Add all non-static method templates to this list.
    ::mstch::array static_methods;
    for (const ::mstch::node &method_node : method_templates)
    {
        const auto method
            = boost::get<std::shared_ptr<::mstch::object>>(method_node);
        const bool is_static = boost::get<bool>(method->at("is_static"));
        if (is_static)
            static_methods.push_back(method);
    }

    return static_methods;
}

::mstch::node CXXRecord::visibleMethods()
{
    ::mstch::array visible_methods;

    // Copy each non-static method into the mstch template.
    std::unordered_set<std::string> non_static_method_names;
    const ::mstch::array non_static_method_templates
        = boost::get<::mstch::array>(methods());
    for (const ::mstch::node &method_node : non_static_method_templates)
    {
        const auto method
            = boost::get<std::shared_ptr<::mstch::object>>(method_node);
        const std::string name = boost::get<std::string>(method->at("name"));
        non_static_method_names.insert(name);

        visible_methods.push_back(method_node);
    }

    // Copy each static method into the mstch template if the static method name
    // doesn't conflict with the non-static method names.
    const ::mstch::array static_method_templates
        = boost::get<::mstch::array>(staticMethods());
    for (const ::mstch::node &method_node : static_method_templates)
    {
        const auto method
            = boost::get<std::shared_ptr<::mstch::object>>(method_node);
        const std::string name = boost::get<std::string>(method->at("name"));

        if (non_static_method_names.find(name) == non_static_method_names.end())
            visible_methods.push_back(method_node);
    }

    return visible_methods;
}

::mstch::node CXXRecord::fields()
{
    ::mstch::array field_templates;

    // Convert each field to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Field>> field_vector;
    for (FieldDecl *const field_decl : decl_->fields())
    {
        if (config_.IsSuppressed(field_decl))
            continue;
        if (field_decl->getAccess() != AS_public)
            continue; // skip protected and private fields

        if (!chimera::util::isCopyable(config_.GetContext(),
                                       field_decl->getType()))
            continue;

        field_vector.push_back(
            std::make_shared<Field>(config_, field_decl, decl_));
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
        if (config_.IsSuppressed(child_decl))
            continue;
        if (!isa<VarDecl>(child_decl))
            continue;

        const VarDecl *static_field_decl = cast<VarDecl>(child_decl);
        if (static_field_decl->getAccess() != AS_public)
            continue;
        if (!static_field_decl->isStaticDataMember())
            continue;

        // Check if a return_value_policy can be generated for this function.
        if (chimera::util::needsReturnValuePolicy(static_field_decl,
                                                  static_field_decl->getType()))
            continue;

        static_field_vector.push_back(
            std::make_shared<Variable>(config_, static_field_decl, decl_));
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

::mstch::node CXXRecord::methodsInternal() const
{
    ::mstch::array method_templates;

    // Convert each method to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Method>> method_vector;
    for (CXXMethodDecl *const method_decl : decl_->methods())
    {
        if (config_.IsSuppressed(method_decl))
            continue;
        if (method_decl->getAccess() != AS_public)
            continue; // skip protected and private members
        if (chimera::util::hasNonPublicParam(method_decl))
            continue;
        if (isa<CXXConversionDecl>(method_decl))
            continue;
        if (isa<CXXDestructorDecl>(method_decl))
            continue;
        if (isa<CXXConstructorDecl>(method_decl))
            continue;
        if (method_decl->isOverloadedOperator())
        {
            const OverloadedOperatorKind kind
                = method_decl->getOverloadedOperator();
            if (!chimera::util::isSupportedOperator(kind))
            {
                std::cerr << "Warning: Skipped operator overloading '"
                          << method_decl->getQualifiedNameAsString()
                          << "' because the operator type is not currently "
                          << "supported by chimera." << std::endl;
                // Suppress unsupported kinds.
                continue;
            }
        }
        if (method_decl->isDeleted())
            continue;
        if (method_decl->getDescribedFunctionTemplate())
            continue;
        if (chimera::util::containsRValueReference(method_decl))
            continue;

        // Skip functions that have incomplete argument types. Boost.Python
        // requires RTTI information about all arguments, including references
        // and pointers.
        if (chimera::util::containsIncompleteType(
                config_.GetCompilerInstance()->getSema(), method_decl))
            continue;

        // Skip functions that have non-copyable argument types.
        // Using these functions requires std::move()-ing their arguments, which
        // we generally cannot do.
        if (chimera::util::containsNonCopyableType(method_decl))
            continue;

        // Generate the method wrapper (but don't add it just yet).
        auto method = std::make_shared<Method>(config_, method_decl, decl_);

        // Check if a return_value_policy can be generated for this function.
        if (::mstch::render("{{return_value_policy}}", method).empty()
            && chimera::util::needsReturnValuePolicy(
                   method_decl, method_decl->getReturnType()))
        {
            // `needsReturnValuePolicy()` already prints an error message,
            // so just continue to the next method if we got here.
            continue;
        }

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

Enum::Enum(const ::chimera::CompiledConfiguration &config, const EnumDecl *decl)
  : ClangWrapper(config, decl)
{
    register_methods(this, {
                               {"type", &Enum::type},
                               {"values", &Enum::values},
                           });
}

::mstch::node Enum::qualifiedName()
{
    if (const YAML::Node node = decl_config_["qualified_name"])
        return node.as<std::string>();

    // In the special case of Enums, the fully-qualified name of the
    // class is pretty much always identical to the type declaration. Since
    // the clang typename resolution works better than the qualified name
    // resolution, we simply use it again here.
    return type();
}

::mstch::node Enum::namespaceScope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateNamespaceScope(config_, nns->getPrefix());
}

::mstch::node Enum::classScope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateClassScope(config_, nns->getPrefix());
}

::mstch::node Enum::scope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateScope(config_, nns->getPrefix());
}

::mstch::node Enum::type()
{
    if (const YAML::Node &node = decl_config_["type"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedDeclTypeAsString(decl_);
}

::mstch::node Enum::values()
{
    ::mstch::array constant_templates;

    // Convert each enum constant to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<EnumConstant>> constant_vector;
    for (const EnumConstantDecl *constant_decl : decl_->enumerators())
    {
        if (config_.IsSuppressed(constant_decl))
            continue;
        constant_vector.push_back(
            std::make_shared<EnumConstant>(config_, constant_decl, decl_));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (constant_vector.size() > 0)
        constant_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto constant_template : constant_vector)
        constant_templates.push_back(constant_template);
    return constant_templates;
}

EnumConstant::EnumConstant(const ::chimera::CompiledConfiguration &config,
                           const EnumConstantDecl *decl,
                           const EnumDecl *enum_decl)
  : ClangWrapper(config, decl), enum_decl_(enum_decl)
{
    // Do nothing.
}

::mstch::node EnumConstant::qualifiedName()
{
    if (const YAML::Node &node = decl_config_["qualified_name"])
        return node.as<std::string>();

    // In the special case of EnumConstants, rather than letting clang try to
    // fully resolve the qualified name, we can simply get it from appending
    // this value to the parent Enum's qualified name.
    auto enumeration = std::make_shared<Enum>(config_, enum_decl_);
    return ::mstch::render("{{type}}", enumeration)
           + "::" + decl_->getNameAsString();
}

Field::Field(const ::chimera::CompiledConfiguration &config,
             const FieldDecl *decl, const CXXRecordDecl *class_decl)
  : ClangWrapper(config, decl), class_decl_(class_decl)
{
    register_methods(this,
                     {
                         {"is_assignable", &Field::isAssignable},
                         {"is_copyable", &Field::isCopyable},
                         {"return_value_policy", &Field::returnValuePolicy},
                     });
}

::mstch::node Field::isAssignable()
{
    if (const YAML::Node &node = decl_config_["is_assignable"])
        return node.as<std::string>();

    return chimera::util::isAssignable(config_.GetContext(), decl_->getType());
}

::mstch::node Field::isCopyable()
{
    if (const YAML::Node &node = decl_config_["is_copyable"])
        return node.as<std::string>();

    return chimera::util::isCopyable(config_.GetContext(), decl_->getType());
}

::mstch::node Field::returnValuePolicy()
{
    // First, check if a return_value_policy was specified for this function.
    if (const YAML::Node &node = decl_config_["return_value_policy"])
        return node.as<std::string>();

    // Extract the value type of this field declaration.
    const QualType value_qual_type = chimera::util::getFullyQualifiedType(
        config_.GetContext(), decl_->getType());

    // Next, check if a return_value_policy is defined on the value type.
    if (const YAML::Node &type_node
        = config_.GetType(value_qual_type)["return_value_policy"])
        return type_node.as<std::string>();

    // Return an empty string if no return value policy exists.
    return std::string{""};
}

::mstch::node Field::qualifiedName()
{
    if (const YAML::Node &node = decl_config_["qualified_name"])
        return node.as<std::string>();

    return chimera::util::getFullyQualifiedDeclTypeAsString(class_decl_)
           + "::" + decl_->getNameAsString();
}

Function::Function(const ::chimera::CompiledConfiguration &config,
                   const FunctionDecl *decl, const CXXRecordDecl *class_decl,
                   const int argument_limit)
  : ClangWrapper(config, decl)
  , class_decl_(class_decl)
  , argument_limit_(argument_limit)
{
    register_methods(
        this,
        {
            {"type", &Function::type},
            {"overloads", &Function::overloads},
            {"params", &Function::params},
            {"params?", &Function::isNonFalse<Function, &Function::params>},
            {"return_type", &Function::returnType},
            {"return_value_policy", &Function::returnValuePolicy},
            {"is_void", &Function::isVoid},
            {"uses_defaults", &Function::usesDefaults},
            {"is_operator", &Function::isOperator},
            {"is_template", &Function::isTemplate},
            {"call", &Function::call},
            {"qualified_call", &Function::qualifiedCall},
        });
}

::mstch::node Function::scope()
{
    return generateScope(config_, decl_->getEnclosingNamespaceContext());
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

    return chimera::util::getFullyQualifiedTypeName(config_.GetContext(),
                                                    pointer_type);
}

::mstch::node Function::overloads()
{
    ::mstch::array overloads;

    // Create a list of wrappers that re-wrap this function with a subset of
    // the full set of arguments (i.e. omitting some of the default arguments).
    auto arg_range = chimera::util::getFunctionArgumentRange(decl_);
    for (unsigned n_args = arg_range.first; n_args < arg_range.second; ++n_args)
    {
        overloads.push_back(
            std::make_shared<Function>(config_, decl_, class_decl_, n_args));
    }

    // Add this function to its own list of overloads.
    // It can be distinguished from other copies because `uses_defaults=false`.
    overloads.push_back(shared_from_this());

    return overloads;
}

::mstch::node Function::params()
{
    ::mstch::array param_templates;

    // Convert each parameter to a template object.
    // Since template objects are lazily-evaluated, this isn't expensive.
    std::vector<std::shared_ptr<Parameter>> param_vector;
    for (unsigned param_idx = 0; param_idx < decl_->getNumParams(); ++param_idx)
    {
        // If argument-limiting is occurring, drop all parameters past the
        // argument limit.
        if (argument_limit_ >= 0)
        {
            if (static_cast<int>(param_vector.size()) >= argument_limit_)
                break;
        }

        // Construct a default name, just in case it is unspecified.
        std::stringstream ss;
        ss << "_arg" << param_idx << "_";

        // Create a parameter template and add it to the parameter array.
        const ParmVarDecl *param_decl = decl_->getParamDecl(param_idx);
        param_vector.push_back(std::make_shared<Parameter>(
            config_, param_decl, decl_, class_decl_, ss.str()));
    }

    // Find and flag the last item.
    // TODO: is there a better way to do this?
    if (param_vector.size() > 0)
        param_vector.back()->setLast(true);

    // Copy each template into the mstch template array.
    for (auto param_template : param_vector)
        param_templates.push_back(param_template);
    return param_templates;
}

::mstch::node Function::returnType()
{
    // First, check if a return_value_policy was specified for this function.
    if (const YAML::Node &node = decl_config_["return_type"])
        return node.as<std::string>();

    // Extract the return type of this function declaration.
    return chimera::util::getFullyQualifiedTypeName(config_.GetContext(),
                                                    decl_->getReturnType());
}

::mstch::node Function::returnValuePolicy()
{
    // First, check if a return_value_policy was specified for this function.
    if (const YAML::Node &node = decl_config_["return_value_policy"])
        return node.as<std::string>();

    // Extract the return type of this function declaration.
    const QualType return_qual_type = chimera::util::getFullyQualifiedType(
        config_.GetContext(), decl_->getReturnType());

    // Next, check if a return_value_policy is defined on the return type.
    if (const YAML::Node &type_node
        = config_.GetType(return_qual_type)["return_value_policy"])
        return type_node.as<std::string>();

    // Return an empty string if no return value policy exists.
    return std::string{""};
}

::mstch::node Function::isVoid()
{
    // First, check if a return_value_policy was specified for this function.
    if (const YAML::Node &node = decl_config_["return_type"])
        return (node.as<std::string>() == "void");

    return decl_->getReturnType()->isVoidType();
}

::mstch::node Function::namespaceScope()
{
    return generateNamespaceScope(config_,
                                  decl_->getEnclosingNamespaceContext());
}

::mstch::node Function::classScope()
{
    return generateClassScope(config_, decl_->getEnclosingNamespaceContext());
}

::mstch::node Function::usesDefaults()
{
    return argument_limit_ >= 0;
}

std::string Function::nameAsString()
{
    if (decl_->isOverloadedOperator())
    {
        return chimera::util::getOperatorName(decl_->getOverloadedOperator());
    }

    return ClangWrapper::nameAsString();
}

::mstch::node Function::qualifiedName()
{
    if (const YAML::Node &node = decl_config_["qualified_name"])
        return node.as<std::string>();

    if (!class_decl_)
        return decl_->getQualifiedNameAsString();

    return chimera::util::getFullyQualifiedDeclTypeAsString(class_decl_)
           + "::" + decl_->getNameAsString();
}

::mstch::node Function::isOperator()
{
    return decl_->isOverloadedOperator();
}

::mstch::node Function::qualifiedCall()
{
    if (const YAML::Node &node = decl_config_["qualified_call"])
        return node.as<std::string>();

    const auto template_str = chimera::util::getTemplateParameterString(
        decl_, config_.GetBindingName());

    // Construct the basic qualified name.
    if (!class_decl_)
        return decl_->getQualifiedNameAsString() + template_str;

    return chimera::util::getFullyQualifiedDeclTypeAsString(class_decl_)
           + "::" + decl_->getNameAsString() + template_str;
}

::mstch::node Function::call()
{
    if (const YAML::Node &node = decl_config_["call"])
        return node.as<std::string>();

    return decl_->getNameAsString()
           + chimera::util::getTemplateParameterString(
                 decl_, config_.GetBindingName());
}

::mstch::node Function::isTemplate()
{
    return decl_->isFunctionTemplateSpecialization();
}

Method::Method(const ::chimera::CompiledConfiguration &config,
               const CXXMethodDecl *decl, const CXXRecordDecl *class_decl)
  : Function(config, decl, class_decl), method_decl_(decl)
{
    register_methods(this, {
                               {"is_const", &Method::isConst},
                               {"is_static", &Method::isStatic},
                               {"is_virtual", &Method::isVirtual},
                               {"is_pure_virtual", &Method::isPureVirtual},
                           });
}

::mstch::node Method::isConst()
{
    return method_decl_->isConst();
}

::mstch::node Method::isStatic()
{
    return method_decl_->isStatic();
}

::mstch::node Method::isVirtual()
{
    return method_decl_->isVirtual();
}

::mstch::node Method::isPureVirtual()
{
    return method_decl_->isPure();
}

Namespace::Namespace(const ::chimera::CompiledConfiguration &config,
                     const NamespaceDecl *decl)
  : ClangWrapper(config, decl)
{
    // Do nothing.
}

::mstch::node Namespace::scope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl());

    return generateScope(config_, nns->getPrefix());
}

Parameter::Parameter(const ::chimera::CompiledConfiguration &config,
                     const ParmVarDecl *decl, const FunctionDecl *method_decl,
                     const CXXRecordDecl *class_decl,
                     const std::string default_name)
  : ClangWrapper(config, decl)
  , method_decl_(method_decl)
  , class_decl_(class_decl)
  , default_name_(default_name)
{
    // Reserved for future use.
    CHIMERA_UNUSED(class_decl_);

    register_methods(this, {
                               {"name", &Parameter::name},
                               {"type", &Parameter::type},
                           });
}

::std::string Parameter::nameAsString()
{
    if (const YAML::Node &node = decl_config_["name"])
        return node.as<std::string>();

    // Ignore argument is part of a variadic function
    // (since it could be non-unique).
    if (const auto template_decl = method_decl_->getPrimaryTemplate())
        if (chimera::util::isVariadicFunctionTemplate(template_decl))
            return default_name_;

    // Use the "default_name" if the name would otherwise be empty
    std::string name = decl_->getNameAsString();
    return (name.empty()) ? default_name_ : name;
}

::mstch::node Parameter::type()
{
    if (const YAML::Node &node = decl_config_["type"])
        return node.as<std::string>();

    auto type_str = chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), decl_->getType());

    if (config_.GetBindingName() == "pybind11")
        type_str = util::stripNoneCopyableEigenWrappers(type_str);
    // FIXME: Replace this Pybind11 specific workaround with a more
    // general solution.

    return type_str;
}

Variable::Variable(const ::chimera::CompiledConfiguration &config,
                   const VarDecl *decl, const CXXRecordDecl *class_decl)
  : ClangWrapper(config, decl), class_decl_(class_decl)
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

    return chimera::util::getFullyQualifiedDeclTypeAsString(class_decl_)
           + "::" + decl_->getNameAsString();
}

::mstch::node Variable::namespaceScope()
{
    return generateNamespaceScope(config_, decl_->getDeclContext());
}

::mstch::node Variable::classScope()
{
    return generateClassScope(config_, decl_->getDeclContext());
}

::mstch::node Variable::scope()
{
    return generateScope(config_, decl_->getDeclContext());
}

::mstch::node Variable::isAssignable()
{
    if (const YAML::Node &node = decl_config_["is_assignable"])
        return node.as<std::string>();

    return chimera::util::isAssignable(config_.GetContext(), decl_->getType());
}

Typedef::Typedef(const CompiledConfiguration &config,
                 const TypedefNameDecl *decl,
                 const CXXRecordDecl *underlying_class_decl)
  : ClangWrapper(config, decl), class_decl_(underlying_class_decl)
{
    register_methods(this, {
                               {"underlying_class", &Typedef::underlyingClass},
                               {"is_builtin_type", &Typedef::isBuiltinType},
                           });
}

::mstch::node Typedef::namespaceScope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateNamespaceScope(config_, nns->getPrefix());
}

::mstch::node Typedef::classScope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateClassScope(config_, nns->getPrefix());
}

::mstch::node Typedef::scope()
{
    const NestedNameSpecifier *nns
        = cling::utils::TypeName::CreateNestedNameSpecifier(
            config_.GetContext(), decl_->getCanonicalDecl(), true);

    return generateScope(config_, nns->getPrefix());
}

::mstch::node Typedef::underlyingClass()
{
    return std::make_shared<CXXRecord>(config_, class_decl_);
}

::mstch::node Typedef::isBuiltinType()
{
    return false;
}

BuiltinTypedef::BuiltinTypedef(const CompiledConfiguration &config,
                               const TypedefNameDecl *decl,
                               const BuiltinType *builtin_type)
  : ClangWrapper(config, decl), builtin_type_(builtin_type)
{
    register_methods(this,
                     {
                         {"is_builtin_type", &BuiltinTypedef::isBuiltinType},
                         {"underlying_type", &BuiltinTypedef::underlyingType},
                     });
}

::mstch::node BuiltinTypedef::isBuiltinType()
{
    return true;
}

::mstch::node BuiltinTypedef::underlyingType()
{
    // Use underlying type to get the declaration of the original type
    const QualType underlying_type = decl_->getUnderlyingType();
    const std::string underlying_type_name
        = chimera::util::getFullyQualifiedTypeName(config_.GetContext(),
                                                   underlying_type);

    return chimera::util::getBuiltinTypeName(underlying_type_name);
}

} // namespace mstch
} // namespace chimera
