#include "chimera/visitor.h"

#include <algorithm>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <boost/algorithm/string/join.hpp>

using namespace chimera;
using namespace clang;
using boost::algorithm::join;

// TODO: Support template functions.
// TODO: Detect missing copy constructors, possibly using:
//  hasUserDeclaredCopyConstructor()
//  hasCopyConstructorWithConstParam ()

chimera::Visitor::Visitor(clang::CompilerInstance *ci,
                          std::unique_ptr<CompiledConfiguration> cc)
: context_(&(ci->getASTContext()))
, config_(std::move(cc))
{
    // Do nothing.
}

bool chimera::Visitor::VisitDecl(Decl *decl)
{
    // Only visit declarations in namespaces we are configured to read.
    if (!IsEnclosed(decl))
        return true;

    if (isa<ClassTemplateDecl>(decl))
        GenerateClassTemplate(cast<ClassTemplateDecl>(decl));
    else if (isa<CXXRecordDecl>(decl))
        GenerateCXXRecord(cast<CXXRecordDecl>(decl));
    else if (isa<EnumDecl>(decl))
        GenerateEnum(cast<EnumDecl>(decl));
    else if (isa<VarDecl>(decl))
        GenerateGlobalVar(cast<VarDecl>(decl));
    else if (isa<FunctionDecl>(decl))
        GenerateGlobalFunction(cast<FunctionDecl>(decl));

    return true;
}

bool chimera::Visitor::GenerateCXXRecord(CXXRecordDecl *const decl)
{
    if (!decl->hasDefinition() || decl->getDefinition() != decl)
        return false;

    const YAML::Node &node = config_->GetDeclaration(decl);

    llvm::raw_pwrite_stream *const stream = config_->GetOutputFile(decl);
    if (!stream)
    {
        std::cerr << "Failed to create output file for '"
                  << decl->getQualifiedNameAsString() << "'." << std::endl;
        return false;
    }

    *stream << "::boost::python::class_<"
            << decl->getTypeForDecl()->getCanonicalTypeInternal().getAsString();

    const YAML::Node &noncopyable_node = node["noncopyable"];
    if (noncopyable_node && noncopyable_node.as<bool>(false))
        *stream << ", ::boost::python::noncopyable";

    if (const YAML::Node &held_type_node = node["held_type"])
        *stream << ", " << held_type_node.as<std::string>();

    std::vector<std::string> base_names;

    if (const YAML::Node &bases_node = node["bases"])
        base_names = bases_node.as<std::vector<std::string> >();
    else
        base_names = GetBaseClassNames(decl);

    if (!base_names.empty())
    {
        *stream << ", ::boost::python::bases<"
                  << join(base_names, ", ") << " >";
    }

    *stream << " >(\"" << decl->getNameAsString()
            << "\", boost::python::no_init)";

    // Methods
    std::set<std::string> overloaded_method_names;

    for (CXXMethodDecl *const method_decl : decl->methods())
    {
        if (method_decl->getAccess() != AS_public)
            continue; // skip protected and private members

        if (isa<CXXConversionDecl>(method_decl))
            ; // do nothing
        else if (isa<CXXDestructorDecl>(method_decl))
            ; // do nothing
        else if (isa<CXXConstructorDecl>(method_decl))
        {
            GenerateCXXConstructor(
                *stream, decl, cast<CXXConstructorDecl>(method_decl));
        }
        else if (method_decl->isOverloadedOperator())
            ; // TODO: Wrap overloaded operators.
        else if (method_decl->isStatic())
        {
            // TODO: Missing the dot.
            if (GenerateFunction(*stream, decl, method_decl))
                overloaded_method_names.insert(method_decl->getNameAsString());
        }
        else
        {
            // TODO: Missing the dot.
            GenerateFunction(*stream, decl, method_decl);
        }
    }

    // Static methods MUST be declared after overloads are defined.
    for (const std::string &method_name : overloaded_method_names)
        *stream << ".staticmethod(\"" << method_name << "\")\n";

    // Fields
    for (FieldDecl *const field_decl : decl->fields())
    {
        if (field_decl->getAccess() != AS_public)
            continue; // skip protected and private fields

        GenerateField(*stream, decl, field_decl);
    }

    for (Decl *const child_decl : decl->decls())
    {
        if (isa<VarDecl>(child_decl))
            GenerateStaticField(*stream, decl, cast<VarDecl>(child_decl));
    }

    *stream << ";\n";
    
    return true;
}

bool chimera::Visitor::GenerateCXXConstructor(
    llvm::raw_pwrite_stream &stream,
    CXXRecordDecl *class_decl,
    CXXConstructorDecl *decl)
{
    std::vector<std::string> argument_types;

    for (ParmVarDecl *param_decl : decl->params())
        argument_types.push_back(param_decl->getType().getAsString());

    stream << ".def(::boost::python::init<"
           << join(argument_types, ", ")
           << ">());\n";
    return true;
}

bool chimera::Visitor::GenerateFunction(
    llvm::raw_pwrite_stream &stream,
    CXXRecordDecl *class_decl, FunctionDecl *decl)
{
    decl = decl->getCanonicalDecl();

    QualType pointer_type;
    if (class_decl)
    {
        pointer_type = context_->getMemberPointerType(
            decl->getType(), class_decl->getTypeForDecl());
    }
    else
    {
        pointer_type = context_->getPointerType(decl->getType());
    }

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
            return false;
        }
        else if (return_type->isPointerType())
        {
            std::cerr
                << "Warning: Skipped method '"
                << decl->getQualifiedNameAsString()
                << "' because it returns a pointer and no"
                   "  'return_value_policy' was specified.\n";
            return false;
        }

        // TODO: Check if return_type is non-copyable.
    }

    if (class_decl)
        stream << ".";

    stream << "def(\"" << decl->getNameAsString() << "\""
           << ", static_cast<" << pointer_type.getAsString() << ">(&"
           << decl->getQualifiedNameAsString() << ")";

    if (rvp_node)
    {
        stream << ", ::boost::python::return_value_policy<"
               << rvp_node.as<std::string>() << " >";
    }

    const auto params = GetParameterNames(decl);
    if (!params.empty())
    {
        // TODO: Supress any default parameters that occur after the first
        // non-default to default transition. This can only occur if evaluating
        // the default value of one or more parameters failed.

        // TODO: Assign names to unnamed arguments.

        std::vector<std::string> python_args;
        for (const auto &param : params)
        {
            std::stringstream python_arg;
            python_arg << "::boost::python::arg(\"" << param.first << "\")";

            if (!param.second.empty())
                python_arg << " = " << param.second;

            python_args.push_back(python_arg.str());
        }

        stream << ", (" << join(python_args, ", ") << ")";
    }

    stream << ")\n";
    return true;
}

bool chimera::Visitor::GenerateField(
    llvm::raw_pwrite_stream &stream,
    clang::CXXRecordDecl *class_decl,
    clang::FieldDecl *decl)
{
    if (decl->getType().isConstQualified())
        stream << ".def_readonly";
    else
        stream << ".def_readwrite";

    // TODO: Check if a copy constructor is defined for this type.

    stream << "(\"" << decl->getNameAsString() << "\","
           << " &" << decl->getQualifiedNameAsString() << ");\n";
    return true;
}

bool chimera::Visitor::GenerateStaticField(
    llvm::raw_pwrite_stream &stream,
    clang::CXXRecordDecl *class_decl,
    clang::VarDecl *decl)
{
    if (decl->getAccess() != AS_public)
        return false;
    else if (!decl->isStaticDataMember())
        return false;

    stream << ".add_static_property(\"" << decl->getNameAsString() << "\", "
           << "[]() { return " << decl->getQualifiedNameAsString() << "; }";

    if (!decl->getType().isConstQualified())
    {
        stream << "[](" << decl->getType().getAsString() << " value) { "
               << decl->getQualifiedNameAsString() << " = value; }";
    }

    stream << ")\n";

    return true;
}

bool chimera::Visitor::GenerateClassTemplate(clang::ClassTemplateDecl *decl)
{
    if (decl != decl->getCanonicalDecl())
        return false;

    for (ClassTemplateSpecializationDecl *spec_decl : decl->specializations())
    {
        if (spec_decl->getSpecializationKind() == TSK_Undeclared)
            continue;

        CXXRecordDecl *decl = spec_decl->getTypeForDecl()->getAsCXXRecordDecl();
        GenerateCXXRecord(decl);
    }

    return true;
}

bool chimera::Visitor::GenerateEnum(clang::EnumDecl *decl)
{
    llvm::raw_pwrite_stream *const stream = config_->GetOutputFile(decl);
    if (!stream)
    {
        std::cerr << "Failed to create output file for '"
                  << decl->getQualifiedNameAsString() << "'." << std::endl;
        return false;
    }

    *stream << "::boost::python::enum_<"
            << decl->getQualifiedNameAsString()
            << ">(\"" << decl->getNameAsString() << "\")\n";

    for (EnumConstantDecl *constant_decl : decl->enumerators())
    {
        *stream << ".value(\"" << constant_decl->getNameAsString() << "\", "
                << constant_decl->getQualifiedNameAsString() << ")\n";
    }

    *stream << ";\n";

    return true;
}

bool chimera::Visitor::GenerateGlobalVar(clang::VarDecl *decl)
{
    if (!decl->isFileVarDecl())
        return false;
    else if (!decl->isThisDeclarationADefinition())
        return false;

    llvm::raw_pwrite_stream *const stream = config_->GetOutputFile(decl);
    if (!stream)
    {
        std::cerr << "Failed to create output file for '"
                  << decl->getQualifiedNameAsString() << "'." << std::endl;
        return false;
    }

    *stream << "::boost::python::scope().attr(\"" << decl->getNameAsString()
            << "\") = " << decl->getQualifiedNameAsString() << ";\n";
    return true;
}

bool chimera::Visitor::GenerateGlobalFunction(clang::FunctionDecl *decl)
{
    if (isa<clang::CXXMethodDecl>(decl))
        return false;
    else if (decl->isOverloadedOperator())
        return false; // TODO: Wrap overloaded operators.

    llvm::raw_pwrite_stream *const stream = config_->GetOutputFile(decl);
    if (!stream)
    {
        std::cerr << "Failed to create output file for '"
                  << decl->getQualifiedNameAsString() << "'." << std::endl;
        return false;
    }

    return GenerateFunction(*stream, nullptr, decl);
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
    chimera::Visitor::GetParameterNames(clang::FunctionDecl *decl) const
{
    std::vector<std::pair<std::string, std::string>> params;

    for (ParmVarDecl *param_decl : decl->params())
    {
        const std::string param_name = param_decl->getNameAsString();
        const Type *param_type = param_decl->getType().getTypePtr();
        std::string param_value;

        if (param_decl->hasDefaultArg()
            && !param_decl->hasUninstantiatedDefaultArg()
            && !param_decl->hasUnparsedDefaultArg())
        {
            Expr *default_expr = param_decl->getDefaultArg();
            assert(default_expr);

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
            else if (default_expr->hasNonTrivialCall(*context_))
            {
                // TODO: How do we print the decl with argument + return types?
                std::cerr
                  << "Warning: Unable to evaluate non-trivial call in default"
                     "  value for parameter"
                  << " '" << param_name << "' of method"
                  << " '" << decl->getQualifiedNameAsString() << "'.\n";
            }
            else
            {
                // TODO: How do we print the decl with argument + return types?
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
