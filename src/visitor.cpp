#include "chimera/visitor.h"
#include "chimera/mstch.h"
#include "chimera/util.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <boost/algorithm/string/join.hpp>
#include <llvm/Support/raw_ostream.h>

using namespace chimera;
using namespace clang;
using boost::algorithm::join;

namespace
{

bool IsInsideTemplateClass(DeclContext *decl_context)
{
    if (!decl_context->isRecord())
        return false;

    if (isa<CXXRecordDecl>(decl_context))
    {
        CXXRecordDecl *record_decl = cast<CXXRecordDecl>(decl_context);
        if (record_decl->getDescribedClassTemplate())
            return true;
    }

    DeclContext *parent_context = decl_context->getParent();
    if (parent_context)
        return IsInsideTemplateClass(parent_context);
    else
        return false;
}

/**
 * Gets a pointer to an enclosing class declaration if one exists.
 * Returns null if there is no enclosing class.
 */
CXXRecordDecl *GetEnclosingClassDecl(DeclContext *decl_context)
{
    DeclContext *parent_context = decl_context->getParent();

    // TODO: replace this with null-ing cast operation.
    return (isa<CXXRecordDecl>(parent_context))
               ? cast<CXXRecordDecl>(parent_context)
               : nullptr;
}

/**
 * Gets a pointer to an enclosing class declaration if one exists.
 * Returns null if there is no enclosing class.
 */
CXXRecordDecl *GetEnclosingClassDecl(Decl *decl)
{
    decl = decl->getCanonicalDecl();

    if (isa<DeclContext>(decl))
    {
        return GetEnclosingClassDecl(cast<DeclContext>(decl));
    }
    else if (isa<VarDecl>(decl))
    {
        return GetEnclosingClassDecl(cast<VarDecl>(decl)->getDeclContext());
    }
    else if (isa<FunctionDecl>(decl))
    {
        return GetEnclosingClassDecl(
            cast<FunctionDecl>(decl)->getEnclosingNamespaceContext());
    }
    return nullptr;
}

} // namespace

chimera::Visitor::Visitor(clang::CompilerInstance *ci,
                          CompiledConfiguration &cc)
  : printing_policy_(ci->getLangOpts()), config_(cc)
{
    // Do nothing.
}

bool chimera::Visitor::shouldVisitTemplateInstantiations() const
{
    return true;
}

bool chimera::Visitor::shouldVisitImplicitCode() const
{
    return true;
}

bool chimera::Visitor::VisitDecl(Decl *decl)
{
    // Only visit declarations in namespaces we are configured to read.
    if (config_.IsSuppressed(decl))
        return true;

    // Only visit declarations whose enclosing classes are generatable.
    auto *enclosing_decl = GetEnclosingClassDecl(decl);
    if (enclosing_decl)
    {
        // If the enclosing class is not yet visited, visit it.
        // (VisitDecl checks for re-visits, so we don't need to do it here.)
        VisitDecl(enclosing_decl);

        // If after visiting, the class was still not created, then do not
        // generate this enclosed definition.
        if (traversed_class_decls_.find(enclosing_decl->getCanonicalDecl())
            == traversed_class_decls_.end())
            return true;
    }

    if (isa<CXXRecordDecl>(decl))
    {
        // Every class template is represented by a CXXRecordDecl and a
        // ClassTemplateDecl. We handle code generation of template classes in
        // the above case, so we don't process them here.
        //
        // This also suppresses inner class that are contained inside a
        // template class.
        auto *class_decl = cast<CXXRecordDecl>(decl);

        if (!IsInsideTemplateClass(class_decl))
        {
            if (GenerateCXXRecord(class_decl))
                traversed_class_decls_.insert(class_decl->getCanonicalDecl());
        }
    }
    else if (isa<EnumDecl>(decl))
    {
        auto *enum_decl = cast<EnumDecl>(decl);
        if (!IsInsideTemplateClass(enum_decl))
            GenerateEnum(cast<EnumDecl>(decl));
    }
    else if (isa<VarDecl>(decl))
    {
        // Variables inside a CXXRecordDecl are handled above. This check is
        // necessary suppress namespace-scope definitions of static member
        // variables, which are required to enable ODR-use of constexpr static
        // member variables.
        //
        // See: http://stackoverflow.com/a/28446388/111426
        if (!isa<CXXRecordDecl>(decl->getDeclContext()))
            GenerateGlobalVar(cast<VarDecl>(decl));
    }
    else if (isa<FunctionDecl>(decl))
    {
        GenerateGlobalFunction(cast<FunctionDecl>(decl));
    }
    else if (isa<NamespaceDecl>(decl))
    {
        // We don't need to generate anything here, but we should
        // tell the compiled configuration that we reached this
        // namespace during traversal.
        config_.AddTraversedNamespace(cast<NamespaceDecl>(decl));
    }
    else if (isa<TypedefNameDecl>(decl))
    {
        GenerateTypedefName(cast<TypedefNameDecl>(decl));
    }

    return true;
}

bool chimera::Visitor::GenerateCXXRecord(CXXRecordDecl *decl)
{
    // Only traverse CXX records that contain the actual class definition.
    if (!decl->hasDefinition())
        return false;
    decl = decl->getDefinition();

    // Avoid generating duplicates of the same class.
    if (traversed_class_decls_.find(decl->getCanonicalDecl())
        != traversed_class_decls_.end())
        return false;

    // Ignore partial template specializations. These still have unbound
    // template parameters.
    if (isa<ClassTemplatePartialSpecializationDecl>(decl))
        return false;

    // Skip protected and private classes.
    if (decl->getAccess() == AS_private || decl->getAccess() == AS_protected)
        return false;

    // Incantations necessary to handle nested template classes.
    // TODO: This likely handles only one level of nesting.
    // See: http://stackoverflow.com/q/35376737/111426
    if (clang::CXXRecordDecl *decl2 = decl->getTemplateInstantiationPattern())
    {
        if (decl2->getAccess() == AS_private
            || decl2->getAccess() == AS_protected)
            return false;

        if (clang::ClassTemplateDecl *decl3
            = decl2->getDescribedClassTemplate())
        {
            if (decl3->getAccess() == AS_private
                || decl3->getAccess() == AS_protected)
                return false;
        }
    }

    // Skip incomplete types. Boost.Python requires RTTI, which requires the
    // complete type.
    if (!decl->isCompleteDefinition())
        return false;

    // Ensure traversal of base classes before this class.
    std::set<const CXXRecordDecl *> base_decls
        = chimera::util::getBaseClassDecls(decl);
    for (auto base_decl : base_decls)
    {
        if (traversed_class_decls_.find(base_decl->getCanonicalDecl())
            == traversed_class_decls_.end())
            VisitDecl(const_cast<CXXRecordDecl *>(base_decl));
    }

    // Serialize using a mstch template.
    auto context = std::make_shared<chimera::mstch::CXXRecord>(
        config_, decl, &traversed_class_decls_);

    // TODO(#148): Workaround to skip generating class binding that is
    // unintentionally generated for lambda function in header. See #148 for the
    // details.
    if (context->typeAsString() == "(lambda)")
        return false;

    return config_.Render(context);
}

bool chimera::Visitor::GenerateEnum(clang::EnumDecl *decl)
{
    // Skip protected and private enums.
    if (decl->getAccess() == AS_private || decl->getAccess() == AS_protected)
        return false;

    // Serialize using a mstch template.
    auto context = std::make_shared<chimera::mstch::Enum>(config_, decl);

    // TODO(#121): Workaround to ignore anonymous enum. The type string of an
    // anonymous enum contains "(anonymous)". See #121 for the details.
    const std::string type = ::mstch::render("{{type}}", context);
    if (type.find("(anonymous)") != std::string::npos)
        return false;

    return config_.Render(context);
}

bool chimera::Visitor::GenerateGlobalVar(clang::VarDecl *decl)
{
    if (!decl->isFileVarDecl())
        return false;
    else if (!decl->isThisDeclarationADefinition())
        return false;
    // Ignore unspecialized template variables.
    else if (decl->getDescribedVarTemplate())
        return false;
    // TODO(#297): Ignore specialized template variables because the current
    // variable mstch template is not able to generate valid binding.
    else if (isa<clang::VarTemplateSpecializationDecl>(decl))
        return false;

    // TODO: Support return_value_policy for global variables.
    if (chimera::util::needsReturnValuePolicy(decl, decl->getType()))
        return false;

    // Serialize using a mstch template.
    auto context = std::make_shared<chimera::mstch::Variable>(config_, decl);
    return config_.Render(context);
}

bool chimera::Visitor::GenerateGlobalFunction(clang::FunctionDecl *decl)
{
    // Only generate functions when we reach their "canonical" definition.
    // This prevents functions from being generated multiple times.
    if (decl != decl->getCanonicalDecl())
        return false;
    // Ignore functions that are actually methods of CXXRecords.
    else if (isa<clang::CXXMethodDecl>(decl))
        return false;
    // Ignore overloaded operators (we can't currently wrap them).
    else if (decl->isOverloadedOperator())
    {
        std::cerr << "Warning: Skipped operator overloading '"
                  << decl->getQualifiedNameAsString()
                  << "' because non-member operators are not currently "
                  << "supported by chimera." << std::endl;
        return false; // TODO: Wrap overloaded operators.
    }
    // Ignore unspecialized template functions.
    else if (decl->getDescribedFunctionTemplate())
        return false;

    // Skip functions that have incomplete argument types. Boost.Python
    // requires RTTI information about all arguments, including references and
    // pointers.
    if (chimera::util::containsIncompleteType(
            config_.GetCompilerInstance()->getSema(), decl))
        return false;

    // Skip functions that have non-copyable argument types passed by value.
    // Using these functions requires std::move()-ing their arguments, which
    // we generally cannot do.
    if (chimera::util::containsNonCopyableType(decl))
        return false;

    // TODO: Support return_value_policy for global functions.
    if (chimera::util::needsReturnValuePolicy(decl, decl->getType()))
        return false;

    // Serialize using a mstch template.
    auto context = std::make_shared<chimera::mstch::Function>(config_, decl);
    return config_.Render(context);
}

bool chimera::Visitor::GenerateTypedefName(TypedefNameDecl *decl)
{
    // TODO: Other derived classes of TypedefNameDecl are not tested
    if (!isa<TypeAliasDecl>(decl) && !isa<TypedefDecl>(decl))
        return false;

    // Use underlying type to get the declaration of the original type
    QualType underlying_type = decl->getUnderlyingType();
    std::string underlying_type_name = chimera::util::getFullyQualifiedTypeName(
        config_.GetContext(), underlying_type);

    // Skip if the defining type is templated
    if (isa<TypeAliasDecl>(decl))
    {
        if (cast<TypeAliasDecl>(decl)->isTemplated())
            return false;
    }

    // TODO: Fix segfault with DependentNameType (e.g., typename T::type).
    // Skip this for now. See #328 for the details.
    if (isa<DependentNameType>(underlying_type))
    {
        std::cerr << "Warning: Skipping dependent name type '"
                  << decl->getQualifiedNameAsString()
                  << "' because it is not supported yet (#328)." << std::endl;
        return false;
    }

    // Create mstch for typedef where the underlying type is a builtin type.
    if (isa<BuiltinType>(underlying_type))
    {
        // TODO: Support more built-in types
        if (underlying_type_name != "double" && underlying_type_name != "float")
            return false;

        auto context = std::make_shared<chimera::mstch::BuiltinTypedef>(
            config_, decl, cast<BuiltinType>(underlying_type));
        return config_.Render(context);
    }

    // Get the declaration from the underlying type
    auto resolved_decl = chimera::util::resolveRecord(
        config_.GetCompilerInstance(), underlying_type_name);

    // Skip if we could not get the declaration, or the declaration was not
    // of a class.
    if (!resolved_decl || !isa<CXXRecordDecl>(resolved_decl))
        return false;

    // Skip if the original type is not traversed
    auto it = traversed_class_decls_.find(cast<CXXRecordDecl>(resolved_decl));
    if (it == traversed_class_decls_.end())
        return false;

    // Create mstch for typedef
    const clang::CXXRecordDecl *underlying_cxx_record_dec = (*it);
    auto context = std::make_shared<chimera::mstch::Typedef>(
        config_, decl, underlying_cxx_record_dec);

    return config_.Render(context);
}
