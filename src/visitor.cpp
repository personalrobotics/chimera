#include "chimera/visitor.h"
#include "chimera/util.h"
#include "external/cling_utils_AST.h"

#include <algorithm>
#include <clang/Lex/Lexer.h>
#include <boost/algorithm/string/join.hpp>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
#include <string>

using namespace chimera;
using namespace clang;
using boost::algorithm::join;

namespace {

bool IsAssignable(CXXRecordDecl *decl)
{
    if (decl->isAbstract())
        return false;
    else if (!decl->hasCopyAssignmentWithConstParam())
        return false;

    for (CXXMethodDecl *method_decl : decl->methods())
        if (method_decl->isCopyAssignmentOperator() && !method_decl->isDeleted())
            return true;

    return false;
}

bool IsQualTypeAssignable(ASTContext &context, QualType qual_type)
{
    if (qual_type.isConstQualified())
        return false;
    else if (CXXRecordDecl *decl = qual_type.getTypePtr()->getAsCXXRecordDecl())
        return IsAssignable(decl);
    else if (qual_type.getTypePtr()->isArrayType())
        return false;
    else
        return qual_type.isTriviallyCopyableType(context);
}

bool IsCopyable(CXXRecordDecl *decl)
{
    if (!decl->hasDefinition())
        return false;
    else if (decl->isAbstract())
        return false;
    else if (!decl->hasCopyConstructorWithConstParam())
        return false;

    for (CXXConstructorDecl *ctor_decl : decl->ctors())
        if (ctor_decl->isCopyConstructor() && !ctor_decl->isDeleted())
            return true;

    return false;
}

bool IsQualTypeCopyable(ASTContext &context, QualType qual_type)
{
    // TODO: Is this logic correct?

    if (CXXRecordDecl *decl = qual_type.getTypePtr()->getAsCXXRecordDecl())
        return IsCopyable(decl);
    else
        return qual_type.isTriviallyCopyableType(context);
}

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

// Generate a safe name to use for a CXXRecordDecl.
//
// This uses a combination of unqualified names, namespace mangling, and
// config overrides to resolve the string name that a binding should use for
// a given C++ class declaration.
std::string ConstructBindingName(
    CXXRecordDecl *decl, ASTContext &context,
    const chimera::CompiledConfiguration &config)
{
    const YAML::Node node = config.GetDeclaration(decl);

    // If a name is specified in the configuration, use that.
    if (!node.IsNull() && node["name"])
        return node["name"].as<std::string>();

    // If this is an anonymous struct, then use the name of its typedef.
    if (TypedefNameDecl *typedef_decl = decl->getTypedefNameForAnonDecl())
        return typedef_decl->getNameAsString();

    // If the class is not a template class, use the unqualified string name.
    if (!isa<ClassTemplateSpecializationDecl>(decl))
        return decl->getNameAsString();

    // If the class is a template, use the mangled string name so that it does
    // not collide with other template instantiations.
    std::string mangled_name;
    llvm::raw_string_ostream mangled_name_stream(mangled_name);
    context.createMangleContext()->mangleName(decl, mangled_name_stream);
    mangled_name = mangled_name_stream.str();

    // Throw a warning that this class name was mangled, because users will
    // probably want to override these names with more sensible ones.
    std::cerr << "Warning: The class '"
              << chimera::util::getFullyQualifiedTypeName(context,
                    QualType(decl->getTypeForDecl(), 0)) << "'"
              << " was bound to the mangled name "
              << "'" << mangled_name << "'"
              << " because the unqualified class name of "
              << "'" << decl->getNameAsString() << "'"
              << " may be ambiguous.\n";

    return mangled_name;
}

bool ContainsIncompleteType(QualType qual_type)
{
    const Type *type = qual_type.getTypePtr();

    // TODO: We're probably missing a few cases here.

    if (isa<PointerType>(type))
    {
        const PointerType *pointer_type = cast<PointerType>(type);
        return ContainsIncompleteType(pointer_type->getPointeeType());
    }
    else if (isa<ReferenceType>(type))
    {
        const ReferenceType *reference_type = cast<ReferenceType>(type);
        return ContainsIncompleteType(reference_type->getPointeeType());
    }
    else
    {
        return type->isIncompleteType();
    }
}

std::vector<std::pair<std::string, std::string>>
GetParameterNames(ASTContext &context, const chimera::CompiledConfiguration &config,
                  clang::FunctionDecl *decl)
{
    std::vector<std::pair<std::string, std::string>> params;

    for (ParmVarDecl *param_decl : decl->params())
    {
        const std::string param_name = param_decl->getNameAsString();
        const Type *param_type = param_decl->getType().getTypePtr();
        std::string source_text = "UNKNOWN";
        std::string param_value;

        // First, try to resolve a string overload.
        const SourceRange source_range = param_decl->getDefaultArgRange();
        const auto char_range = CharSourceRange::getTokenRange(source_range);
        bool is_invalid = false;
        const StringRef source_text_ref = clang::Lexer::getSourceText(
            char_range, context.getSourceManager(), context.getLangOpts(),
            &is_invalid);

        if (!is_invalid)
        {
            source_text = source_text_ref.str();
            param_value = config.GetConstant(source_text);
        }

        // Otherwise, try to evaluate the expression.
        if (param_value.empty()
            && param_decl->hasDefaultArg()
            && !param_decl->hasUninstantiatedDefaultArg()
            && !param_decl->hasUnparsedDefaultArg())
        {
            Expr *default_expr = param_decl->getDefaultArg();
            assert(default_expr);

            Expr::EvalResult result;
            bool success;

            if (param_type->isReferenceType())
                success = default_expr->EvaluateAsLValue(result, context);
            else
                success = default_expr->EvaluateAsRValue(result, context);

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
                        context, param_decl->getType());
                }
            }
            else if (default_expr->hasNonTrivialCall(context))
            {
                // TODO: How do we print the decl with argument + return types?
                std::cerr
                  << "Warning: Unable to evaluate non-trivial call '"
                  << source_text << "' in default"
                     " value for parameter"
                  << " '" << param_name << "' of method"
                  << " '" << decl->getQualifiedNameAsString() << "'.\n";
            }
            else
            {
                // TODO: How do we print the decl with argument + return types?
                std::cerr
                  << "Warning: Failed to evaluate default value '"
                  << source_text << "' for parameter"
                  << " '" << param_name << "' of method"
                  << " '" << decl->getQualifiedNameAsString() << "'.\n";
            }
        }

        // If a constant has been overriden, use the override instead of the
        // original value.  Currently, this is just done via string-matching.
        param_value = config.GetConstant(param_value);
        params.push_back(std::make_pair(param_name, param_value));
    }

    return params;
}

void GenerateFunctionArguments(
    ASTContext &context, const chimera::CompiledConfiguration &config,
    FunctionDecl *decl, bool leading_comma, chimera::Stream &stream)
{
    // Construct a list of the arguments that are provided to this function,
    // and define named arguments for them based on their c++ names.
    const auto params = GetParameterNames(context, config, decl);

    if (!params.empty())
    {
        // Find the last empty parameter, or return the beginning of the list.
        // This gets us an iterator to parameters that can have defaults, while
        // suppressing defaults that occurred before a default that we failed
        // to evaluate.
        const auto last_blank_param = find_if(params.rbegin(), params.rend(),
            [](const std::pair<std::string, std::string> &param) {
                return param.second.empty();
            });
        const auto param_defaults_begin = last_blank_param.base();

        // TODO: Assign names to unnamed arguments.

        std::vector<std::string> python_args;
        bool use_param_default = false;

        for (auto param_it = params.begin(); param_it != params.end(); ++param_it)
        {
            std::stringstream python_arg;

            // Add the named argument directive.
            python_arg << "::boost::python::arg(\"" << param_it->first << "\")";

            // If this is the first default param, set flag to start serializing them.
            if (param_defaults_begin == param_it)
                use_param_default = true;

            // If we reached the parameters with default arguments, write them.
            if (use_param_default)
                python_arg << " = " << param_it->second;

            // Record this entry to the list of parameter declarations.
            python_args.push_back(python_arg.str());
        }

        if (leading_comma)
            stream << ", ";

        stream << "(" << join(python_args, ", ") << ")";
    }
}

std::string getFullyQualifiedDeclTypeAsString(
    const ASTContext &context, const TypeDecl *decl)
{
    return chimera::util::getFullyQualifiedTypeName(
        context, QualType(decl->getTypeForDecl(), 0));
}

bool needsReturnValuePolicy(
    const ASTContext &context, NamedDecl *decl, const Type *return_type)
{
    if (return_type->isReferenceType())
    {
        std::cerr
            << "Warning: Skipped method '"
            << decl->getQualifiedNameAsString()
            << "' because it returns a reference of type '"
            << chimera::util::getFullyQualifiedTypeName(
                  context, QualType(return_type, 0))
            << "' and no 'return_value_policy' was specified.\n";
        return true;
    }
    else if (return_type->isPointerType())
    {
        std::cerr
            << "Warning: Skipped method '"
            << decl->getQualifiedNameAsString()
            << "' because it returns a pointer of type '"
            << chimera::util::getFullyQualifiedTypeName(
                  context, QualType(return_type, 0))
            << "'and no 'return_value_policy' was specified.\n";
        return true;
    }
    return false;
}

} // namespace

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
    if (!IsEnclosed(decl))
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

    const YAML::Node &node = config_->GetDeclaration(decl);
    if (node.IsNull())
        return false; // Explicitly suppressed.

    // Use GetBaseClassNames to traverse base classes before this class and, in
    // the process, generate their bindings (if necessary).
    const std::vector<std::string> default_base_names = GetBaseClassNames(decl);

    // Open a stream object unique to this CXX record's mangled name.
    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // Get configuration, and use any overrides if they exist.
    if (config_->DumpOverride(decl, *stream))
        return true;

    // Update the Python scope for new declaration.
    if (!GenerateScope(*stream, decl))
      return false; // Parent scope was suppressed.

    *stream << "::boost::python::class_<"
            << chimera::util::getFullyQualifiedTypeName(
                *context_, QualType(decl->getTypeForDecl(), 0));

    const bool is_noncopyable = !IsCopyable(decl);
    const YAML::Node &noncopyable_node = node["noncopyable"];
    if (noncopyable_node.as<bool>(is_noncopyable))
        *stream << ", ::boost::noncopyable";

    if (const YAML::Node &held_type_node = node["held_type"])
        *stream << ", " << held_type_node.as<std::string>();

    std::vector<std::string> base_names
        = node["bases"].as<std::vector<std::string> >(default_base_names);
    if (!base_names.empty())
    {
        *stream << ", ::boost::python::bases<"
                  << join(base_names, ", ") << " >";
    }

    *stream << " >(\"" << ConstructBindingName(decl, *context_, *config_)
            << "\", boost::python::no_init)\n";

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
    chimera::Stream &stream,
    CXXRecordDecl *class_decl,
    CXXConstructorDecl *decl)
{
    decl = decl->getCanonicalDecl();

    if (decl->isDeleted())
        return false;
    else if (decl->isCopyOrMoveConstructor())
        return false;
    else if (class_decl->isAbstract())
        return false;

    std::vector<std::string> argument_types;

    for (ParmVarDecl *param_decl : decl->params())
    {
        if (param_decl->getType().getTypePtr()->isRValueReferenceType())
        {
            std::cerr
                << "Warning: Skipped constructor of "
                << class_decl->getNameAsString()
                << " because parameter '" << param_decl->getNameAsString()
                << "' is an rvalue reference.\n";
            return false;
        }

        argument_types.push_back(
           chimera::util::getFullyQualifiedTypeName(
              *context_, param_decl->getType()));
    }

    stream << ".def(::boost::python::init<"
           << join(argument_types, ", ")
           << ">(";

    GenerateFunctionArguments(*context_, *config_, decl, false, stream);

    stream << "))\n";

    return true;
}

bool chimera::Visitor::GenerateFunction(
    chimera::Stream &stream,
    CXXRecordDecl *class_decl, FunctionDecl *decl)
{
    decl = decl->getCanonicalDecl();

    if (decl->isDeleted())
        return false;

    // Skip function template declarations.
    if (decl->getDescribedFunctionTemplate())
        return false;

    // Skip functions that have incomplete argument types. Boost.Python
    // requires RTTI information about all arguments, including references and
    // pointers.
    for (const ParmVarDecl *param : decl->params())
    {
        if (ContainsIncompleteType(param->getOriginalType()))
        {
            std::cerr
                << "Warning: Skipped function "
                << decl->getQualifiedNameAsString()
                << " because argument '"
                << param->getNameAsString()
                << "' has an incomplete type.\n";
            return false;
        }
    }

    // Get configuration, and use any overrides if they exist.
    if (config_->DumpOverride(decl, stream))
        return true;

    const YAML::Node &node = config_->GetDeclaration(decl);
    if (node.IsNull())
        return false; // Explicitly suppressed.

    // Extract the pointer type of this function declaration.
    QualType pointer_type;
    if (class_decl && !cast<CXXMethodDecl>(decl)->isStatic())
    {
        pointer_type = context_->getMemberPointerType(
            decl->getType(), class_decl->getTypeForDecl());
    }
    else
    {
        pointer_type = context_->getPointerType(decl->getType());
    }
    pointer_type = chimera::util::getFullyQualifiedType(*context_,
                                                        pointer_type);

    // Extract the return type of this function declaration.
    const QualType return_qual_type = 
        chimera::util::getFullyQualifiedType(*context_,
                                             decl->getReturnType());
    const Type *return_type = return_qual_type.getTypePtr();

    // First, check if a return_value_policy was specified for this function.
    std::string return_value_policy
        = node["return_value_policy"].as<std::string>("");

    // Next, check if a return_value_policy is defined on the return type.
    if (return_value_policy.empty())
    {
        const YAML::Node type_node = config_->GetType(return_qual_type);

        // Suppress this return type.
        if (type_node.IsNull())
          return false;

        return_value_policy
            = type_node["return_value_policy"].as<std::string>("");
    }

    // Finally, try the default return_value_policy.
    if (return_value_policy.empty()
     && needsReturnValuePolicy(*context_, decl, return_type))
        return false;

    // Suppress any functions that take arguments by rvalue reference.
    for (const ParmVarDecl *const parm_decl : decl->parameters()) {
        if (parm_decl->getType().getTypePtr()->isRValueReferenceType()) {
            std::cerr
                << "Warning: Skipped method "
                << decl->getQualifiedNameAsString()
                << " because parameter '" << parm_decl->getNameAsString()
                << "' is an rvalue reference.\n";
            return false;
        }
    }

    // If we are inside a class declaration, this is being called within a
    // builder pattern and will start with '.' since it is a member function.
    if (class_decl)
        stream << ".";
    else
        stream << "boost::python::";

    // Create the actual function declaration here using its name and its
    // full pointer reference.
    stream << "def(\"" << decl->getNameAsString() << "\""
           << ", static_cast<"
           << chimera::util::getFullyQualifiedTypeName(*context_, pointer_type)
           << ">(&";

    if (class_decl) {
        stream << getFullyQualifiedDeclTypeAsString(*context_, class_decl)
               << "::" << decl->getNameAsString();
    } else {
        stream << decl->getQualifiedNameAsString();
    }

    stream << ")";

    // If a return value policy was specified, insert it after the function.
    if (!return_value_policy.empty())
    {
        stream << ", ::boost::python::return_value_policy<"
               << return_value_policy << " >()";
    }

    // Generate named arguments.
    GenerateFunctionArguments(*context_, *config_, decl, true, stream);

    stream << ")\n";

    if (!class_decl)
        stream << ";";

    return true;
}

bool chimera::Visitor::GenerateField(
    chimera::Stream &stream,
    clang::CXXRecordDecl *class_decl,
    clang::FieldDecl *decl)
{
    // TODO: Add support for YAML overrides.

    // TODO: Add support for return_value_policy if set in the YAML override,
    // even if this is false. If a return_value_policy is specified we'll have
    // to use add_property() with make_getter and/or make_setter instead of
    // def_readonly or def_readwrite.
    if (!IsQualTypeCopyable(*context_, decl->getType()))
        return false;

    if (IsQualTypeAssignable(*context_, decl->getType()))
        stream << ".def_readwrite";
    else
        stream << ".def_readonly";

    stream << "(\"" << decl->getNameAsString() << "\","
           << " &" << getFullyQualifiedDeclTypeAsString(*context_, class_decl)
           << "::" << decl->getNameAsString()
           << ")\n";
    return true;
}

bool chimera::Visitor::GenerateStaticField(
    chimera::Stream &stream,
    clang::CXXRecordDecl *class_decl,
    clang::VarDecl *decl)
{
    // TODO: How should we handle AS_none here.
    if (decl->getAccess() == AS_private || decl->getAccess() == AS_protected)
        return false;
    if (!decl->isStaticDataMember())
        return false;

    // TODO: Add support for YAML overrides, including return_value_policy.

    // Finally, try the default return_value_policy.
    if (needsReturnValuePolicy(*context_, decl, decl->getType().getTypePtr()))
        return false;

    stream << ".add_static_property(\"" << decl->getNameAsString()
           << "\", ::boost::python::make_getter("
           << getFullyQualifiedDeclTypeAsString(*context_, class_decl)
           << "::" << decl->getNameAsString()
           << ")";

    if (IsQualTypeAssignable(*context_, decl->getType()))
    {
        stream << ", ::boost::python::make_setter("
          << getFullyQualifiedDeclTypeAsString(*context_, class_decl)
          << "::" << decl->getNameAsString()
          << ")";
    }

    stream << ")\n";

    return true;
}

bool chimera::Visitor::GenerateEnum(clang::EnumDecl *decl)
{
    // Skip protected and private enums.
    if ((decl->getAccess() == AS_private) || (decl->getAccess() == AS_protected))
        return false;

    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // Update the Python scope for new declaration.
    if (!GenerateScope(*stream, decl))
      return false; // Parent scope was suppressed.

    *stream << "::boost::python::enum_<"
            << getFullyQualifiedDeclTypeAsString(*context_, decl)
            << ">(\"" << decl->getNameAsString() << "\")\n";

    for (EnumConstantDecl *constant_decl : decl->enumerators())
    {
        *stream << ".value(\"" << constant_decl->getNameAsString() << "\", "
                << getFullyQualifiedDeclTypeAsString(*context_, decl)
                << "::" << constant_decl->getNameAsString()
                << ")\n";
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
    // Static data members inside a class templates will incorrectly generate a
    // function call.
    else if (decl->isStaticDataMember())
        return false;

    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // TODO: Support return_value_policy for global variables.
    if (needsReturnValuePolicy(*context_, decl, decl->getType().getTypePtr()))
        return false;

    // Update the Python scope for new declaration.
    if (!GenerateGlobalScope(*stream, decl->getDeclContext()))
        return false; // Parent scope was suppressed.

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

    auto stream = config_->GetOutputFile(decl);
    if (!stream)
        return false;

    // Update the Python scope for new declaration.
    if (!GenerateGlobalScope(*stream, decl->getEnclosingNamespaceContext()))
        return false; // Parent scope was suppressed.

    return GenerateFunction(*stream, nullptr, decl);
}

bool chimera::Visitor::GenerateScope(
    chimera::Stream &stream, TagDecl *decl)
{
  NestedNameSpecifier *const nns
    = cling::utils::TypeName::CreateNestedNameSpecifier(
        *context_, decl, true);

  return GenerateScope(stream, nns->getPrefix());
}

bool chimera::Visitor::GenerateScope(
    chimera::Stream &stream, NestedNameSpecifier *nns)
{
  const std::set<const clang::NamespaceDecl *> &base_namespaces
   = config_->GetNamespaces();

  // Build a list of NestedNameeSpecifiers starting at the root.
  std::vector<NestedNameSpecifier *> namespaces;

  while (nns)
  {
    namespaces.push_back(nns);
    nns = nns->getPrefix();
  }

  std::reverse(std::begin(namespaces), std::end(namespaces));

  // Generate Boost.Python scopes in forward order.
  std::vector<std::string> module_names;

  for (NestedNameSpecifier *const nns : namespaces)
  {
    switch (nns->getKind())
    {
    case NestedNameSpecifier::Namespace:
    {
      // Skip root namespaces.
      NamespaceDecl *decl = nns->getAsNamespace()->getCanonicalDecl();
      if (!base_namespaces.count(decl))
      {
        config_->DumpNamespace(nns);
        module_names.push_back(nns->getAsNamespace()->getNameAsString());
      }
      break;
    }

    case NestedNameSpecifier::TypeSpec:
    {
      CXXRecordDecl *decl = nns->getAsType()->getAsCXXRecordDecl();
      if (!decl)
        throw std::runtime_error("TypeSpec is not a CXXRecordDecl.");

      // Suppress this class if a parent scope was suppressed.
      const std::string binding_name = ConstructBindingName(
        decl, *context_, *config_);
      if (binding_name.empty())
        return false;

      // Generate outer classes before inner classes.
      if (!IsInsideTemplateClass(decl))
      {
          if (GenerateCXXRecord(decl))
              traversed_class_decls_.insert(decl->getCanonicalDecl());
      }

      module_names.push_back(decl->getNameAsString());
      break;
    }

    case NestedNameSpecifier::Global:
    case NestedNameSpecifier::Identifier:
    case NestedNameSpecifier::NamespaceAlias:
    case NestedNameSpecifier::Super:
    case NestedNameSpecifier::TypeSpecWithTemplate:
      throw std::runtime_error("Unsupported type of NestedNameSpecifier.");

    default:
      throw std::runtime_error("Unknown type of NestedNameSpecifier.");
    }
  }

  // TODO: I don't know why this has to be done on a separate line. Putting
  // this inline in the boost::python::scope doesn't create a new scope.
  stream << "::boost::python::object parent_object("
            "::boost::python::scope()";

  for (const std::string &module_name : module_names)
    stream << ".attr(\"" << module_name << "\")";

  stream << ");\n"
            "::boost::python::scope parent_scope(parent_object);\n\n";

  return true;
}

bool chimera::Visitor::GenerateGlobalScope(
  chimera::Stream &stream, clang::DeclContext *decl)
{
    if (!decl)
        return false;

    auto namespace_decl = llvm::dyn_cast_or_null<NamespaceDecl>(decl);
    if (!namespace_decl)
        throw std::runtime_error("DeclContext is not a NamespaceDecl.");

    clang::NestedNameSpecifier *nns
      = cling::utils::TypeName::CreateNestedNameSpecifier(
          *context_, namespace_decl);

    return GenerateScope(stream, nns);
}

std::vector<std::string> chimera::Visitor::GetBaseClassNames(
    CXXRecordDecl *decl)
{
    std::vector<std::string> base_names;

    for (CXXBaseSpecifier &base_decl : decl->bases())
    {
        if (base_decl.getAccessSpecifier() != AS_public)
            continue;

        // TODO: Filter out transitive base classes.

        CXXRecordDecl *const base_record_decl
            = base_decl.getType()->getAsCXXRecordDecl();
        if (!base_record_decl)
        {
            std::cerr << "Warning: Omitted base class of class "
                      << chimera::util::getFullyQualifiedTypeName(*context_,
                            QualType(decl->getTypeForDecl(), 0))
                      << "' because it is not a CXXRecordDecl.\n";
            continue;
        }

        const QualType base_record_type(base_record_decl->getTypeForDecl(), 0);

        // Generate the base class, if necessary.
        VisitDecl(base_record_decl);

        const bool base_exists
            = traversed_class_decls_.count(base_record_decl->getCanonicalDecl());
        const std::string base_name
            = chimera::util::getFullyQualifiedTypeName(*context_, base_record_type);

        if (base_exists)
            base_names.push_back(base_name);
        else
        {
            std::cerr << "Warning: Omitted base class '" << base_name
                      << "' of class '"
                      << chimera::util::getFullyQualifiedTypeName(*context_,
                            QualType(decl->getTypeForDecl(), 0))
                      << "' because it could not be wrapped.\n";
        }
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
