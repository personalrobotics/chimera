#include "chimera/util.h"
#include "cling_utils_AST.h"

#include <iostream>
#include <sstream>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Mangle.h>
#include <clang/Parse/Parser.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/SemaDiagnostic.h>
#include "clang/AST/DeclTemplate.h"

namespace chimera
{
namespace util
{

using namespace clang;

namespace
{
/**
 * Empty QualType used when returning a type-resolution failure.
 */
static const QualType emptyType_;

/**
 * Generates incremental typedef names to avoid namespace conflicts.
 *
 * This is used by functions that need to compile a temporary typedef to
 * resolve a type within the AST.
 */
std::string generateUniqueName()
{
    // Use a static variable to generate non-duplicate names.
    static unsigned counter = 0;

    std::stringstream ss;
    ss << "chimera_placeholder_" << (counter++);
    return ss.str();
}

} // namespace

::mstch::node wrapYAMLNode(const YAML::Node &node, ScalarConversionFn fn)
{
    switch (node.Type())
    {
        case YAML::NodeType::Scalar:
            return fn ? fn(node) : node.as<std::string>();
        case YAML::NodeType::Sequence:
        {
            ::mstch::array context;
            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
            {
                const YAML::Node &value = *it;
                context.emplace_back(wrapYAMLNode(value));
            }
            return context;
        }
        case YAML::NodeType::Map:
        {
            ::mstch::map context;
            for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
            {
                const std::string name = it->first.as<std::string>();
                const YAML::Node &value = it->second;
                context[name] = wrapYAMLNode(value);
            }
            return context;
        }
        case YAML::NodeType::Undefined:
        case YAML::NodeType::Null:
        default:
            return nullptr;
    }
}

void extendWithYAMLNode(::mstch::map &map, const YAML::Node &node,
                        bool overwrite, ScalarConversionFn fn)
{
    // Ignore invalid node.
    if (!node)
        return;

    // Ignore non-map types of YAML::Node.
    if (!node.IsMap())
        return;

    // Add entries from the YAML configuration directly into the object.
    // This wraps each YAML node in a recursive conversion wrapper.
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
    {
        const std::string name = it->first.as<std::string>();
        const YAML::Node &value = it->second;

        // Insert the key if overwriting or if the key does not exist.
        // (This uses emplace's behavior of returning a std::pair of which
        //  the second element indicates whether an insertion was done or
        //  if the element already existed to avoid two lookups.)
        auto result = map.emplace(std::make_pair(name, ::mstch::node{}));
        if (result.second || overwrite)
        {
            result.first->second = wrapYAMLNode(value, fn);
        }
    }
}

const NamedDecl *resolveDeclaration(CompilerInstance *ci,
                                    const llvm::StringRef declStr)
{
    // Immediately return if passed an empty string.
    if (declStr.empty())
        return nullptr;

    // Create a new parser to handle this type parsing.
    Preprocessor &preprocessor = ci->getPreprocessor();
    Sema &sema = ci->getSema();
    Parser parser(preprocessor, sema, /* SkipFunctionBodies = */ false);

    // Set up the preprocessor to only care about incrementally handling type.
    preprocessor.getDiagnostics().setIgnoreAllWarnings(true);
    preprocessor.getDiagnostics().setSeverityForGroup(
        diag::Flavor::WarningOrError, "out-of-line-declaration",
        diag::Severity::Ignored);
    preprocessor.enableIncrementalProcessing();
    const_cast<LangOptions &>(preprocessor.getLangOpts()).SpellChecking = 0;

    // Put the type string into a buffer and run it through the preprocessor.
    FileID fid = sema.getSourceManager().createFileID(
        llvm::MemoryBuffer::getMemBufferCopy(
            declStr.str() + ";", "chimera.util.resolveDeclaration"));
    preprocessor.EnterSourceFile(fid, /* DirLookup = */ 0, SourceLocation());
    parser.Initialize();

    // Try parsing the type name.
    Parser::DeclGroupPtrTy ADecl;
    while (!parser.ParseTopLevelDecl(ADecl))
    {
        // A DeclGroupRef may have multiple Decls, so we iterate through each
        // one.
        // TODO: We probably shouldn't actually iterate here.
        if (ADecl)
        {
            DeclGroupRef DG = ADecl.get();
            for (auto i = DG.begin(), e = DG.end(); i != e; ++i)
            {
                Decl *D = (*i)->getCanonicalDecl();
                return (isa<NamedDecl>(D)) ? cast<NamedDecl>(D) : nullptr;
            }
        }
    }

    std::cerr << "Failed to parse '" << declStr.str() << "'." << std::endl;
    return nullptr;
}

const QualType resolveType(CompilerInstance *ci, const llvm::StringRef typeStr)
{
    auto decl = resolveDeclaration(
        ci, "typedef " + typeStr.str() + " " + generateUniqueName());
    if (!decl)
        return emptyType_;

    if (!isa<TypedefDecl>(decl))
    {
        std::cerr << "Expected 'typedef' declaration, found '"
                  << decl->getNameAsString() << "'." << std::endl;
        return emptyType_;
    }

    auto typedef_decl = cast<TypedefDecl>(decl);
    return typedef_decl->getUnderlyingType().getCanonicalType();
}

const RecordDecl *resolveRecord(CompilerInstance *ci,
                                const llvm::StringRef recordStr)
{
    auto type = resolveType(ci, recordStr).getTypePtrOrNull();
    if (!type)
        return nullptr;

    auto cxx_record_type = type->getAsCXXRecordDecl();
    if (!cxx_record_type)
        return nullptr;

    return cast<RecordDecl>(cxx_record_type->getCanonicalDecl());
}

class TemplateDeclString
{
public:
    TemplateDeclString() = default;
    TemplateDeclString(const std::string &str)
      : origianl_str_(str), is_valid_(false)
    {
        tokenize(str);
    }

    const std::string &get_decl_string() const
    {
        return origianl_str_;
    }

    bool isValid() const
    {
        return is_valid_;
    }

    const std::string &get_class_name() const
    {
        return class_name_;
    }

    const std::vector<std::string> &get_template_param_types() const
    {
        return template_param_types_;
    }

    const std::vector<std::string> &get_template_param_names() const
    {
        return template_param_names_;
    }

    const std::string &get_template_params_decl() const
    {
        return template_params_decl_;
    }

private:
    void tokenize(const std::string &inputStr)
    {
        // TODO: template template parameters are not supported (yet)

        static const std::string template_keword = "template";
        static const std::string whitespaces = " \t\n\r\f\v";

        const std::string str = trim(inputStr);
        std::string::size_type curr = 0;

        template_param_types_.clear();
        template_param_names_.clear();

        // Fail if the string doesn't start with "template" keyword
        if (!startsWith(str, template_keword))
        {
            is_valid_ = false;
            return;
        }

        // Set curr to the first non-whitespace character after "template"
        // keyword
        curr = str.find_first_not_of(whitespaces, template_keword.size());
        if (curr == std::string::npos || str[curr] != '<')
        {
            is_valid_ = false;
            return;
        }

        // Find template parameters
        int depth = 0;
        curr = str.find_first_not_of(whitespaces, curr + 1);
        auto token_begin = curr;
        while (curr != str.size() && curr != std::string::npos)
        {
            const auto &c = str[curr];

            if (c == '<')
            {
                depth++;
            }
            else if (c == ',')
            {
                if (depth == 0)
                {
                    if (!parse_template_param(str, curr, token_begin,
                                              whitespaces))
                    {
                        is_valid_ = false;
                        return;
                    }
                }
            }
            else if (c == '>')
            {
                if (depth == 0)
                {
                    template_params_decl_ = str.substr(0, curr + 1);
                    if (!parse_template_param(str, curr, token_begin,
                                              whitespaces))
                    {
                        is_valid_ = false;
                        return;
                    }
                    break;
                }

                depth--;
            }

            curr++;
        }

        // The numbers of left brackets and right brackets should be the same
        if (depth > 0)
        {
            is_valid_ = false;
            return;
        }

        // Class name
        class_name_ = str.substr(curr, str.size() - curr);

        is_valid_ = true;
    }

    bool parse_template_param(const std::string &str,
                              std::string::size_type &curr,
                              std::string::size_type &token_begin,
                              const std::string &whitespaces)
    {
        auto token_end = str.find_first_of(whitespaces, token_begin);
        template_param_types_.push_back(
            str.substr(token_begin, token_end - token_begin));

        token_begin = str.find_first_not_of(whitespaces, token_end);
        token_end = str.find_first_of(whitespaces + ">" + ",", curr);
        template_param_names_.push_back(
            str.substr(token_begin, token_end - token_begin));

        curr = str.find_first_not_of(whitespaces, curr + 1);
        token_begin = curr;
        return true;
    }

    std::string origianl_str_;
    std::string template_params_decl_;
    std::vector<std::string> template_param_types_;
    std::vector<std::string> template_param_names_;
    std::string class_name_;
    bool is_valid_;
};

std::string makeTypeAliasTemplateString(const std::string &declStr)
{
    auto parsed_decl_str = TemplateDeclString(declStr);
    if (!parsed_decl_str.isValid())
        return declStr;

    std::stringstream ss;
    ss << parsed_decl_str.get_template_params_decl();
    ss << " using ";
    ss << generateUniqueName();
    ss << " = ";
    ss << parsed_decl_str.get_class_name();
    ss << "<";
    const auto &param_names = parsed_decl_str.get_template_param_names();
    for (auto i = 0u; i < param_names.size(); ++i)
    {
        ss << param_names[i];
        if (i != param_names.size() - 1u)
            ss << ", ";
    }
    ss << ">;";

    return ss.str();
}

const clang::ClassTemplateDecl *resolveClassTemplate(
    CompilerInstance *ci, const llvm::StringRef recordStr)
{
    auto type_alias_template_str = makeTypeAliasTemplateString(recordStr.str());

    auto decl = resolveDeclaration(ci, type_alias_template_str);
    if (!decl)
    {
        std::cerr << "Failed to parse following template type alias:\n\n"
                  << type_alias_template_str << std::endl;
        return nullptr;
    }

    auto type_alias_template_decl = dyn_cast<TypeAliasTemplateDecl>(decl);
    if (!type_alias_template_decl)
    {
        std::cerr << "Expected type alias template declaration, found '"
                  << decl->getNameAsString() << "'." << std::endl;
        return nullptr;
    }

    TypeAliasDecl *type_alias_decl
        = type_alias_template_decl->getTemplatedDecl();
    QualType underlying_type
        = type_alias_decl->getUnderlyingType().getCanonicalType();
    const TemplateSpecializationType *template_specialization_type
        = dyn_cast<TemplateSpecializationType>(underlying_type);
    TemplateDecl *template_decl
        = template_specialization_type->getTemplateName().getAsTemplateDecl();

    return dyn_cast<ClassTemplateDecl>(template_decl->getCanonicalDecl());
}

const NamespaceDecl *resolveNamespace(CompilerInstance *ci,
                                      const llvm::StringRef nsStr)
{
    auto decl = resolveDeclaration(
        ci, "namespace " + generateUniqueName() + " = " + nsStr.str());
    if (!decl)
        return nullptr;

    if (!isa<NamespaceAliasDecl>(decl))
    {
        std::cerr << "Expected 'namespace' alias declaration, found '"
                  << decl->getNameAsString() << "'." << std::endl;
        return nullptr;
    }

    return cast<NamespaceAliasDecl>(decl)->getNamespace()->getCanonicalDecl();
}

std::string constructMangledName(const NamedDecl *decl)
{
    std::string mangled_name;
    llvm::raw_string_ostream mangled_name_stream(mangled_name);
    decl->getASTContext().createMangleContext()->mangleName(
        decl, mangled_name_stream);
    return mangled_name_stream.str();
}

std::string constructBindingName(const CXXRecordDecl *decl)
{
    // If this is an anonymous struct, then use the name of its typedef.
    if (TypedefNameDecl *typedef_decl = decl->getTypedefNameForAnonDecl())
        return typedef_decl->getNameAsString();

    // If the class is not a template class, use the unqualified string name.
    if (!isa<ClassTemplateSpecializationDecl>(decl))
        return decl->getNameAsString();

    // If the class is a template, use the mangled string name so that it does
    // not collide with other template instantiations.
    std::string mangled_name = constructMangledName(decl);

    // Throw a warning that this class name was mangled, because users will
    // probably want to override these names with more sensible ones.
    std::cerr << "Warning: The class '"
              << getFullyQualifiedDeclTypeAsString(decl) << "'"
              << " was bound to the mangled name "
              << "'" << mangled_name << "'"
              << " because the unqualified class name of "
              << "'" << decl->getNameAsString() << "'"
              << " may be ambiguous.\n";

    return mangled_name;
}

QualType getFullyQualifiedType(ASTContext &context, QualType qt)
{
    return cling::utils::TypeName::GetFullyQualifiedType(qt, context);
}

std::string getFullyQualifiedTypeName(ASTContext &context, QualType qt)
{
    return cling::utils::TypeName::GetFullyQualifiedName(qt, context);
}

std::string getFullyQualifiedDeclTypeAsString(const TypeDecl *decl)
{
    return getFullyQualifiedTypeName(decl->getASTContext(),
                                     QualType(decl->getTypeForDecl(), 0));
}

bool isAssignable(const CXXRecordDecl *decl)
{
    if (decl->isAbstract())
        return false;
    else if (!decl->hasCopyAssignmentWithConstParam())
        return false;

    for (CXXMethodDecl *method_decl : decl->methods())
        if (method_decl->isCopyAssignmentOperator()
            && !method_decl->isDeleted())
            return true;

    return false;
}

bool isAssignable(ASTContext &context, QualType qual_type)
{
    if (qual_type.isConstQualified())
        return false;
    else if (CXXRecordDecl *decl = qual_type.getTypePtr()->getAsCXXRecordDecl())
        return isAssignable(decl);
    else if (qual_type.getTypePtr()->isArrayType())
        return false;
    else
        return qual_type.isTriviallyCopyableType(context);
}

bool isCopyable(const CXXRecordDecl *decl)
{
    if (decl->isAbstract())
        return false;
    else if (!decl->hasCopyConstructorWithConstParam())
        return false;

    for (CXXConstructorDecl *ctor_decl : decl->ctors())
        if (ctor_decl->isCopyConstructor() && !ctor_decl->isDeleted())
            return true;

    return false;
}

bool isCopyable(ASTContext &context, QualType qual_type)
{
    // TODO: Is this logic correct?

    if (CXXRecordDecl *decl = qual_type.getTypePtr()->getAsCXXRecordDecl())
        return isCopyable(decl);
    else
        return qual_type.isTriviallyCopyableType(context);
}

bool isInsideTemplateClass(const DeclContext *decl_context)
{
    if (!decl_context->isRecord())
        return false;

    if (isa<CXXRecordDecl>(decl_context))
    {
        const CXXRecordDecl *record_decl
            = cast<const CXXRecordDecl>(decl_context);
        if (record_decl->getDescribedClassTemplate())
            return true;
    }

    const DeclContext *parent_context = decl_context->getParent();
    if (parent_context)
        return isInsideTemplateClass(parent_context);
    else
        return false;
}

bool isVariadicFunctionTemplate(const FunctionTemplateDecl *decl)
{
    // Based on SemaTemplateDeduction.cpp
    // http://clang.llvm.org/doxygen/SemaTemplateDeduction_8cpp_source.html
    FunctionDecl *function_decl = decl->getTemplatedDecl();

    unsigned param_idx = function_decl->getNumParams();
    if (param_idx == 0)
        return false;

    ParmVarDecl *last = function_decl->getParamDecl(param_idx - 1);
    if (!last->isParameterPack())
        return false;

    // Make sure that no previous parameter is a parameter pack.
    while (--param_idx > 0)
    {
        if (function_decl->getParamDecl(param_idx - 1)->isParameterPack())
            return false;
    }

    return true;
}

std::vector<std::string> getTemplateParameterStrings(
    ASTContext &context, const ArrayRef<TemplateArgument> &params)
{
    std::vector<std::string> outputs;

    for (const TemplateArgument &param : params)
    {
        switch (param.getKind())
        {
            case TemplateArgument::Type:
                outputs.push_back(util::getFullyQualifiedTypeName(
                    context, param.getAsType()));
                break;

            case TemplateArgument::NullPtr:
                outputs.push_back("nullptr");
                break;

            case TemplateArgument::Integral:
                outputs.push_back(param.getAsIntegral().toString(10));
                break;

            case TemplateArgument::Pack:
            {
                const auto pack_strs = getTemplateParameterStrings(
                    context, param.getPackAsArray());
                outputs.insert(outputs.end(), pack_strs.begin(),
                               pack_strs.end());
                break;
            }
            case TemplateArgument::Template:
            case TemplateArgument::TemplateExpansion:
                std::cerr
                    << "Template argument is a template-template argument;"
                    << " this is not supported.\n";
                return std::vector<std::string>();

            case TemplateArgument::Declaration:
            case TemplateArgument::Expression:
            case TemplateArgument::Null:
                std::cerr << "Template argument is not fully resolved.\n";
                return std::vector<std::string>();

            default:
                std::cerr << "Unknown kind of template argument.\n";
                return std::vector<std::string>();
        }
    }
    return outputs;
}

std::string getTemplateParameterString(const FunctionDecl *decl)
{
    std::stringstream ss;

    // If this is a template, add the template arguments to the end.
    if (decl->isFunctionTemplateSpecialization())
    {
        if (const TemplateArgumentList *const params
            = decl->getTemplateSpecializationArgs())
        {
            ss << "<";
            const auto param_strs = getTemplateParameterStrings(
                decl->getASTContext(), params->asArray());

            for (size_t iparam = 0; iparam < param_strs.size(); ++iparam)
            {
                ss << param_strs[iparam];
                if (iparam < param_strs.size() - 1)
                    ss << ", ";
            }
            ss << ">";
        }
    }

    return ss.str();
}

std::set<const CXXRecordDecl *> getBaseClassDecls(const CXXRecordDecl *decl)
{
    std::set<const CXXRecordDecl *> base_decls;

    for (const CXXBaseSpecifier &base_decl : decl->bases())
    {
        if (base_decl.getAccessSpecifier() != AS_public)
            continue;

        // TODO: Filter out transitive base classes.

        base_decls.insert(base_decl.getType()->getAsCXXRecordDecl());
    }

    return base_decls;
}

std::set<const CXXRecordDecl *> getBaseClassDecls(
    const CXXRecordDecl *decl, std::set<const CXXRecordDecl *> available_decls)
{
    // Get all base classes.
    std::set<const CXXRecordDecl *> all_base_decls = getBaseClassDecls(decl);

    // Filter the classes by availability.
    std::set<const CXXRecordDecl *> base_decls;
    for (const CXXRecordDecl *base_decl : all_base_decls)
    {
        const bool base_available = available_decls.count(base_decl);

        if (base_available)
            base_decls.insert(base_decl);
        else
        {
            std::cerr << "Warning: Omitted base class '"
                      << base_decl->getNameAsString() << "' of class '"
                      << decl->getNameAsString()
                      << "' because it could not be wrapped.\n";
        }
    }

    return base_decls;
}

bool containsIncompleteType(Sema &sema, QualType qual_type)
{
    const Type *type = qual_type.getTypePtr();

    // TODO: We're probably missing a few cases here.

    if (isa<PointerType>(type))
    {
        const PointerType *pointer_type = cast<PointerType>(type);
        return containsIncompleteType(sema, pointer_type->getPointeeType());
    }
    else if (isa<ReferenceType>(type))
    {
        const ReferenceType *reference_type = cast<ReferenceType>(type);
        return containsIncompleteType(sema, reference_type->getPointeeType());
    }
    else if (type->isVoidPointerType())
    {
        return false;
    }
    else if (type->isVoidType())
    {
        return false;
    }
    else
    {
        return sema.RequireCompleteType({}, qual_type, 0);
    }
}

bool containsIncompleteType(Sema &sema, const FunctionDecl *decl)
{
    for (unsigned int iparam = 0; iparam < decl->getNumParams(); ++iparam)
    {
        const ParmVarDecl *const param_decl = decl->getParamDecl(iparam);
        if (containsIncompleteType(sema, param_decl->getOriginalType()))
        {
            std::cerr << "Warning: Skipped function '"
                      << decl->getQualifiedNameAsString() << "' because";

            if (param_decl->getNameAsString().empty())
                std::cerr << " the parameter at index " << iparam;
            else
                std::cerr << " parameter '" << param_decl->getNameAsString()
                          << "'";

            std::cerr << " has an incomplete type '"
                      << getFullyQualifiedTypeName(decl->getASTContext(),
                                                   param_decl->getType())
                      << "'.\n";
            return true;
        }
    }
    return false;
}

bool containsRValueReference(const FunctionDecl *decl)
{
    for (unsigned int iparam = 0; iparam < decl->getNumParams(); ++iparam)
    {
        const ParmVarDecl *const param_decl = decl->getParamDecl(iparam);
        if (param_decl->getType().getTypePtr()->isRValueReferenceType())
        {
            std::cerr << "Warning: Skipped function '"
                      << decl->getQualifiedNameAsString() << "' because";

            if (param_decl->getNameAsString().empty())
                std::cerr << " the parameter at index " << iparam;
            else
                std::cerr << " parameter '" << param_decl->getNameAsString()
                          << "'";

            std::cerr << " is an rvalue reference of type '"
                      << getFullyQualifiedTypeName(decl->getASTContext(),
                                                   param_decl->getType())
                      << "'.\n";

            return true;
        }
    }
    return false;
}

bool containsNonCopyableType(const FunctionDecl *decl)
{
    for (unsigned int iparam = 0; iparam < decl->getNumParams(); ++iparam)
    {
        const ParmVarDecl *const param_decl = decl->getParamDecl(iparam);
        const clang::Type *const param_type
            = param_decl->getType().getTypePtr();

        if (!param_type->isReferenceType() && !param_type->isPointerType()
            && !isCopyable(decl->getASTContext(), param_decl->getType()))
        {
            std::cerr << "Warning: Skipped function '"
                      << decl->getQualifiedNameAsString() << "' because";

            if (param_decl->getNameAsString().empty())
                std::cerr << " the parameter at index " << iparam;
            else
                std::cerr << " parameter '" << param_decl->getNameAsString()
                          << "'";

            std::cerr << " has a non-copyable type '"
                      << getFullyQualifiedTypeName(decl->getASTContext(),
                                                   param_decl->getType())
                      << "'.\n";

            return true;
        }
    }
    return false;
}

bool needsReturnValuePolicy(const NamedDecl *decl, QualType return_type)
{
    if (return_type.getTypePtr()->isReferenceType())
    {
        std::cerr << "Warning: Skipped method '"
                  << decl->getQualifiedNameAsString()
                  << "' because it returns the reference type '"
                  << getFullyQualifiedTypeName(decl->getASTContext(),
                                               return_type)
                  << "' and no 'return_value_policy' was specified.\n";
        return true;
    }
    else if (return_type.getTypePtr()->isPointerType())
    {
        std::cerr << "Warning: Skipped method '"
                  << decl->getQualifiedNameAsString()
                  << "' because it returns the pointer type '"
                  << getFullyQualifiedTypeName(decl->getASTContext(),
                                               return_type)
                  << "' and no 'return_value_policy' was specified.\n";
        return true;
    }
    return false;
}

std::pair<unsigned, unsigned> getFunctionArgumentRange(
    const clang::FunctionDecl *decl)
{
    const unsigned max_arguments = decl->getNumParams();

    unsigned min_arguments = 0;
    for (; min_arguments < max_arguments; ++min_arguments)
    {
        if (decl->getParamDecl(min_arguments)->hasDefaultArg())
            break;
    }

    return std::pair<unsigned, unsigned>(min_arguments, max_arguments);
}

bool hasNonPublicParam(const CXXMethodDecl *decl)
{
    for (auto i = 0u; i < decl->getNumParams(); ++i)
    {
        const ParmVarDecl *param_decl = decl->getParamDecl(i);
        QualType param_type = param_decl->getType();
        if (param_type->isReferenceType())
        {
            param_type = param_type.getNonReferenceType();
        }
        // TODO: are there any other special cases than reference type?

        if (auto record_type = dyn_cast<RecordType>(param_type))
        {
            CXXRecordDecl *cxx_record_decl
                = cast<CXXRecordDecl>(record_type->getDecl());
            if (cxx_record_decl->getAccess() != AS_public)
            {
                return true;
            }
        }
    }
    return false;
}

std::string trimRight(std::string s, const char *t)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

std::string trimLeft(std::string s, const char *t)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

std::string trim(std::string s, const char *t)
{
    return trimLeft(trimRight(s, t), t);
}

bool startsWith(const std::string &str, const std::string &prefix)
{
    return ((prefix.size() <= str.size())
            && std::equal(prefix.begin(), prefix.end(), str.begin()));
}

} // namespace util
} // namespace chimera
