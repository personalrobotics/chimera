#include "chimera/mstch.h"
#include "chimera/util.h"
#include "chimera/visitor.h"

// TODO: Clean this up and move to something other than a header.
#include "chimera/boost_python_mstch.h"

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

    // Skip incomplete types. Boost.Python requires RTTI, which requires the
    // complete type.
    if (!decl->isCompleteDefinition())
        return false;

    // Ignore declarations that have been explicitly suppressed.
    const YAML::Node &node = config_->GetDeclaration(decl);
    if (node.IsNull())
        return false;

    // Open a stream object unique to this CXX record's mangled name.
    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // Get configuration, and use any overrides if they exist.
    if (config_->DumpOverride(decl, *stream))
        return true;

    // Serialize using a mstch template.
    ::mstch::map context; // TODO: globals from config.
    context["class"] = std::make_shared<chimera::mstch::CXXRecord>(*config_, decl);
    *stream << ::mstch::render(CLASS_BINDING_CPP, context);
    return true;
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

    // Open a stream object unique to this Enum's mangled name.
    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // Get configuration, and use any overrides if they exist.
    if (config_->DumpOverride(decl, *stream))
        return true;

    // Serialize using a mstch template.
    ::mstch::map context; // TODO: globals from config.
    context["enum"] = std::make_shared<chimera::mstch::Enum>(*config_, decl);
    *stream << ::mstch::render(CLASS_BINDING_CPP, context);
    return true;
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

    // Open a stream object unique to this variable's mangled name.
    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // Get configuration, and use any overrides if they exist.
    if (config_->DumpOverride(decl, *stream))
        return true;

    // Serialize using a mstch template.
    ::mstch::map context; // TODO: globals from config.
    context["variable"] = std::make_shared<chimera::mstch::Variable>(*config_, decl);
    *stream << ::mstch::render(VAR_BINDING_CPP, context);
    return true;
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

    // Open a stream object unique to this function's mangled name.
    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // Get configuration, and use any overrides if they exist.
    if (config_->DumpOverride(decl, *stream))
        return true;

    // Serialize using a mstch template.
    ::mstch::map context; // TODO: globals from config.
    context["function"] = std::make_shared<chimera::mstch::Function>(*config_, decl);
    *stream << ::mstch::render(FUNCTION_BINDING_CPP, context);
    return true;
}
