#include "chimera/visitor.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <boost/algorithm/string/join.hpp>

using namespace chimera;
using namespace clang;
using boost::algorithm::join;

chimera::Visitor::Visitor(clang::CompilerInstance *ci,
                          std::unique_ptr<CompiledConfiguration> cc)
: context_(&(ci->getASTContext()))
, config_(std::move(cc))
{
    // Do nothing.
}

bool chimera::Visitor::VisitDecl(Decl *decl)
{
    if (!decl->isCanonicalDecl())
        return true;

    if (!IsEnclosed(decl))
        return true;

    if (isa<CXXRecordDecl>(decl))
        GenerateCXXRecord(cast<CXXRecordDecl>(decl));

    return true;
}

void chimera::Visitor::GenerateCXXRecord(CXXRecordDecl *const decl)
{
    const YAML::Node &node = config_->GetDeclaration(decl);

    std::cout
        << "::boost::python::class_<"
        << decl->getQualifiedNameAsString();

    const YAML::Node &noncopyable_node = node["noncopyable"];
    if (const bool noncopyable = noncopyable_node && noncopyable_node.as<bool>(false))
    {
        std::cout << ", ::boost::python::noncopyable";
    }

    if (const YAML::Node &held_type_node = node["held_type"])
    {
        std::cout << ", " << held_type_node.as<std::string>();
    }

    std::vector<std::string> base_names;

    if (const YAML::Node &bases_node = node["bases"])
        base_names = bases_node.as<std::vector<std::string> >();
    else
        base_names = GetBaseClassNames(decl);

    if (!base_names.empty())
    {
        std::cout << ", ::boost::python::bases<"
                  << join(base_names, ", ") << " >";
    }

    std::cout << " >" << std::endl;
}

std::vector<std::string> chimera::Visitor::GetBaseClassNames(
    CXXRecordDecl *decl) const
{
    std::vector<std::string> base_names;

    for (CXXBaseSpecifier &base_decl : decl->bases())
    {
        if (base_decl.getAccessSpecifier() != AS_public)
            continue;

        // TODO: Filter out transitive base classes.

        CXXRecordDecl *const base_record_decl
          = base_decl.getType()->getAsCXXRecordDecl();
        base_names.push_back(base_record_decl->getQualifiedNameAsString());
    }

    return base_names;
}

bool chimera::Visitor::IsEnclosed(Decl *decl) const
{
    // Filter over the namespaces and only traverse ones that are enclosed
    // by one of the configuration namespaces.
    for (const auto &it : config_->GetNamespaces())
    {
        if (decl->getDeclContext() && it->Encloses(decl->getDeclContext()))
        {
            return true;
        }
    }
    return false;
}
