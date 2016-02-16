#include "chimera/mstch.h"
#include "chimera/util.h"
#include "chimera/visitor.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <string>

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

}

chimera::Visitor::Visitor(clang::CompilerInstance *ci,
                          std::unique_ptr<CompiledConfiguration> cc)
: context_(&(ci->getASTContext()))
, printing_policy_(ci->getLangOpts())
, config_(std::move(cc))
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
    if (!config_->IsEnclosed(decl))
        return true;

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
        GenerateGlobalVar(cast<VarDecl>(decl));
    else if (isa<FunctionDecl>(decl))
        GenerateGlobalFunction(cast<FunctionDecl>(decl));

    return true;
}

bool chimera::Visitor::GenerateCXXRecord(CXXRecordDecl *decl)
{
    // Only traverse CXX records that contain the actual class definition.
    if (!decl->hasDefinition())
        return false;
    decl = decl->getDefinition();

    // Avoid generating duplicates of the same class.
    if (traversed_class_decls_.count(decl->getCanonicalDecl()))
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

      if (clang::ClassTemplateDecl *decl3 = decl2->getDescribedClassTemplate())
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

    // Ignore declarations that have been explicitly suppressed.
    const YAML::Node &node = config_->GetDeclaration(decl);
    if (node.IsNull())
        return false;

    // Ensure traversal of base classes before this class.
    std::set<const CXXRecordDecl *> base_decls =
        chimera::util::getBaseClassDecls(decl);
    for(auto base_decl : base_decls)
    {
        if (traversed_class_decls_.find(base_decl->getCanonicalDecl())
                != traversed_class_decls_.end())
            VisitDecl(const_cast<CXXRecordDecl *>(base_decl));
    }

    // Serialize using a mstch template.
    auto context = std::make_shared<chimera::mstch::CXXRecord>(*config_, decl,
                                                               &traversed_class_decls_);
    return config_->Render(context);
}

bool chimera::Visitor::GenerateEnum(clang::EnumDecl *decl)
{
    // Skip protected and private enums.
    if ((decl->getAccess() == AS_private) || (decl->getAccess() == AS_protected))
        return false;

    // Ignore declarations that have been explicitly suppressed.
    const YAML::Node &node = config_->GetDeclaration(decl);
    if (node.IsNull())
        return false;

    // Serialize using a mstch template.
    auto context = std::make_shared<chimera::mstch::Enum>(*config_, decl);
    return config_->Render(context);
}

bool chimera::Visitor::GenerateGlobalVar(clang::VarDecl *decl)
{
    if (!decl->isFileVarDecl())
        return false;
    else if (!decl->isThisDeclarationADefinition())
        return false;

    // Ignore declarations that have been explicitly suppressed.
    const YAML::Node &node = config_->GetDeclaration(decl);
    if (node.IsNull())
        return false;

    // TODO: Support return_value_policy for global variables.
    if (chimera::util::needsReturnValuePolicy(decl, decl->getType().getTypePtr()))
        return false;

    // Serialize using a mstch template.
	auto context = std::make_shared<chimera::mstch::Variable>(*config_, decl);
    return config_->Render(context);
}

bool chimera::Visitor::GenerateGlobalFunction(clang::FunctionDecl *decl)
{
    if (isa<clang::CXXMethodDecl>(decl))
        return false;
    else if (decl->isOverloadedOperator())
        return false; // TODO: Wrap overloaded operators.

    // Ignore declarations that have been explicitly suppressed.
    const YAML::Node &node = config_->GetDeclaration(decl);
    if (node.IsNull())
        return false;

    // TODO: Support return_value_policy for global functions.
    if (chimera::util::needsReturnValuePolicy(decl, decl->getType().getTypePtr()))
        return false;

    // Serialize using a mstch template.
    auto context = std::make_shared<chimera::mstch::Function>(*config_, decl);
    return config_->Render(context);
}
