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
    if (!decl->hasDefinition())
        return;

    const YAML::Node &node = config_->GetDeclaration(decl);

    std::cout
        << "::boost::python::class_<"
        << decl->getQualifiedNameAsString();

    const YAML::Node &noncopyable_node = node["noncopyable"];
    if (const bool noncopyable = noncopyable_node && noncopyable_node.as<bool>(false))
        std::cout << ", ::boost::python::noncopyable";

    if (const YAML::Node &held_type_node = node["held_type"])
        std::cout << ", " << held_type_node.as<std::string>();

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

    std::cout << " >\n";

    for (CXXMethodDecl *method_decl : decl->methods())
    {
        if (isa<CXXConversionDecl>(method_decl))
            ; // do nothing
        else if (isa<CXXDestructorDecl>(method_decl))
            ; // do nothing
        else if (isa<CXXConstructorDecl>(method_decl))
            ; // TODO: Wrap constructors.
        else if (method_decl->isOverloadedOperator())
            ; // TODO: Wrap overloaded operators.
        else if (method_decl->isStatic())
            ; // TODO: Wrap static functions
        else
            GenerateCXXMethod(decl, method_decl);
    }
}

void chimera::Visitor::GenerateCXXMethod(
    CXXRecordDecl *class_decl, CXXMethodDecl *decl)
{
    decl = decl->getCanonicalDecl();

    const QualType pointer_type = context_->getMemberPointerType(
        decl->getType(), class_decl->getTypeForDecl());
    const Type *return_type = decl->getReturnType().getTypePtr();

    const YAML::Node &node = config_->GetDeclaration(decl);
    const YAML::Node &rvp_node = node["return_value_policy"];

    if (!rvp_node)
    {
        if (return_type->isReferenceType())
        {
            std::cerr
                << "Warning: Skipped method '"
                << decl->getQualifiedNameAsString()
                << "' because it returns a reference and no"
                   "  'return_value_policy' was specified.\n";
            return;
        }
        else if (return_type->isPointerType())
        {
            std::cerr
                << "Warning: Skipped method '"
                << decl->getQualifiedNameAsString()
                << "' because it returns a pointer and no"
                   "  'return_value_policy' was specified.\n";
            return;
        }

        // TODO: CHeck if return_type is non-copyable.
    }

    std::cout
        << ".def(\"" << decl->getNameAsString() << "\""
        << ", static_cast<" << pointer_type.getAsString() << ">(&"
        << decl->getQualifiedNameAsString() << ")";

    if (rvp_node)
    {
        std::cout << ", boost::python::return_value_policy<"
                  << rvp_node.as<std::string>() << " >";
    }

    const auto params = GetParameterNames(decl);
    if (!params.empty())
    {
        std::vector<std::string> python_args;
        for (const auto &param : params)
        {
            std::stringstream python_arg;
            python_arg << "boost::python::arg(\"" << param.first << "\")";

            if (!param.second.empty())
                python_arg << " = " << param.second;

            python_args.push_back(python_arg.str());
        }

        std::cout << ", (" << join(python_args, ", ") << ")";
    }

    std::cout << ")\n";

    //context_->getPointerType(decl->getType()).dump();
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

std::vector<std::pair<std::string, std::string>>
    chimera::Visitor::GetParameterNames(clang::CXXMethodDecl *decl) const
{
    std::vector<std::pair<std::string, std::string>> params;

    for (ParmVarDecl *param_decl : decl->params())
    {
        const std::string param_name = param_decl->getNameAsString();
        const Type *param_type = param_decl->getType().getTypePtr();
        std::string param_value;

        if (param_decl->hasDefaultArg())
        {
            Expr *default_expr = param_decl->getDefaultArg();
            Expr::EvalResult result;
            bool success;

            if (param_type->isReferenceType())
                success = default_expr->EvaluateAsLValue(result, *context_);
            else
                success = default_expr->EvaluateAsRValue(result, *context_);

            if (success)
            {
                param_value = result.Val.getAsString(
                    *context_, param_decl->getType());
            }
            else
            {
                std::cerr
                  << "Warning: Failed to evaluate default value for parameter"
                  << " '" << param_name << "' of method"
                  << " '" << decl->getQualifiedNameAsString() << "'.\n";
            }
        }

        params.push_back(std::make_pair(param_name, param_value));
    }

    return params;
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
