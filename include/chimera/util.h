#ifndef __CHIMERA_UTIL_H__
#define __CHIMERA_UTIL_H__

#include <set>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/CompilerInstance.h>
#include <mstch/mstch.hpp>
#include <yaml-cpp/yaml.h>

namespace chimera
{
namespace util
{

/**
 * Wrapper that generates context for a YAML node.
 *
 * This function automatically converts known datatypes from YAML::Node entries
 * to mstch context entries.  It optionally takes a scalar conversion function
 * which will be applied to scalars before they are re-wrapped by mstch.
 */
using ScalarConversionFn = std::function<std::string(const YAML::Node &)>;
::mstch::node wrapYAMLNode(const YAML::Node &node,
                           ScalarConversionFn fn = nullptr);

/**
 * Adds entries from a YAML::Node to an existing mstch::map.
 *
 * This function takes an existing mstch::map and inserts entries from a
 * YAML::Node node if it is a map.  It can be used to create a mstch::node
 * combining auto-generated properties with ones from a configuration file.
 *
 * When `overwrite` is true, entries that already exist in the mstch::map
 * will be replaced by entries in the YAML::Node if it contains them.  If
 * false, existing properties will not be modified.
 */
void extendWithYAMLNode(::mstch::map &map, const YAML::Node &node,
                        bool overwrite = false,
                        ScalarConversionFn fn = nullptr);

/**
 * Resolve a declaration string within the scope of a compiler instance.
 *
 * This function parses the provided declaration string as a single line of
 * the form `[declStr];` within the AST that is currently loaded by the
 * provided compiler instance.
 *
 * If it can be resolved to a named declaration, the canonical clang::Decl
 * pointer associated with the declaration will be returned, otherwise NULL.
 */
const clang::NamedDecl *resolveDeclaration(clang::CompilerInstance *ci,
                                           const llvm::StringRef declStr);

/**
 * Resolve a record string within the scope of a compiler instance.
 *
 * This function parses the provided record name string as a single line of
 * the form `typedef [recordStr] <uid>;` within the AST that is currently
 * loaded by the provided compiler instance.
 *
 * If it is resolved to a record declaration, the canonical clang::RecordDecl
 * pointer associated with the declaration will be returned, otherwise NULL.
 */
const clang::RecordDecl *resolveRecord(clang::CompilerInstance *ci,
                                       const llvm::StringRef recordStr);

/**
 * Resolve a type string within the scope of a compiler instance.
 *
 * This function parses the provided record name string as a single line of
 * the form `typedef [recordStr] <uid>;` within the AST that is currently
 * loaded by the provided compiler instance.
 *
 * If it is resolved to a qualified type declaration, the canonical
 * clang::QualType associated with the declaration will be returned, otherwise
 * an empty clang::QualType is returned (use getTypeOrNull() to check this).
 */
const clang::QualType resolveType(clang::CompilerInstance *ci,
                                  const llvm::StringRef typeStr);

/**
 * Resolve a namespace string within the scope of a compiler instance.
 *
 * This function parses the provided namespace string as a single line of the
 * form `namespace [nsStr] {};` within the AST that is currently loaded by the
 * provided compiler instance.
 *
 * If it is resolved to a namespace declaration, the canonical
 * clang::NamespaceDecl pointer associated with the namespace will be
 * returned, otherwise NULL.
 */
const clang::NamespaceDecl *resolveNamespace(clang::CompilerInstance *ci,
                                             const llvm::StringRef nsStr);

/**
 * Convert the type into one with fully qualified template parameters.
 *
 * Internally uses cling::utils::getFullyQualifiedType().
 */
clang::QualType getFullyQualifiedType(clang::ASTContext &context,
                                      clang::QualType qt);

/**
 * Get the fully qualified name for a type.
 * This includes full qualification of all template parameters, etc.
 *
 * Internally uses cling::utils::getFullyQualifiedName().
 */
std::string getFullyQualifiedTypeName(clang::ASTContext &context,
                                      clang::QualType qt);

/**
 * Get the fully qualified name for the type used in a type declaration.
 * This includes full qualification of all template parameters, etc.
 */
std::string getFullyQualifiedDeclTypeAsString(const clang::TypeDecl *decl);

/**
 * Get the base CXX record declarations for a CXXRecordDecl.
 *
 * This filters over all the base record entries for the given declaration
 * and returns the public entries. A set of 'available' decls can be
 * provided, in which case only base decls that exist in this set will be
 * returned.
 */
std::set<const clang::CXXRecordDecl *> getBaseClassDecls(
    const clang::CXXRecordDecl *decl);
std::set<const clang::CXXRecordDecl *> getBaseClassDecls(
    const clang::CXXRecordDecl *decl,
    std::set<const clang::CXXRecordDecl *> available_decls);

/**
 * Generate a safe name to use for a CXXRecordDecl.
 *
 * This uses a combination of unqualified names, namespace mangling, and
 * config overrides to resolve the string name that a binding should use for
 * a given C++ class declaration.
 */
std::string constructBindingName(const clang::CXXRecordDecl *decl);

/**
 * Generate the C++ mangled name for a class.
 *
 * This name is generated from the Clang compiler name mangler.
 */
std::string constructMangledName(const clang::NamedDecl *decl);

/**
 * Returns whether a type contains incomplete argument types.
 *
 * This is useful in cases where we need RTTI information about all arguments,
 * including references and pointers.
 */
bool containsIncompleteType(clang::Sema &sema, clang::QualType qual_type);

/**
 * Returns whether any function parameters contain incomplete argument types.
 *
 * This is useful in cases where we need RTTI information about all arguments,
 * including references and pointers.
 */
bool containsIncompleteType(clang::Sema &sema, const clang::FunctionDecl *decl);

/**
 * Returns whether any function parameters contain RValue references.
 */
bool containsRValueReference(const clang::FunctionDecl *decl);

/**
 * Returns whether any function parameters contain a non-copyable type.
 */
bool containsNonCopyableType(const clang::FunctionDecl *decl);

/**
 * Determine if a CXXRecordDecl is referring to a type that could be assigned.
 */
bool isAssignable(const clang::CXXRecordDecl *decl);

/**
 * Determine if a QualType is referring to a type that could be assigned.
 */
bool isAssignable(clang::ASTContext &context, clang::QualType qual_type);

/**
 * Determine if a CXXRecordDecl is referring to a class that is copyable.
 */
bool isCopyable(const clang::CXXRecordDecl *decl);

/**
 * Determine if a QualType is referring to a type that is copyable.
 */
bool isCopyable(clang::ASTContext &context, clang::QualType qual_type);

/**
 * Determine if a declaration is within the context of a template class.
 */
bool isInsideTemplateClass(const clang::DeclContext *decl_context);

/**
 * Determines if a function template declaration is variadic.
 */
bool isVariadicFunctionTemplate(const clang::FunctionTemplateDecl *decl);

/**
 * Convert a list of template arguments to a vector of std::strings.
 *
 * Returns an empty list and prints a warning if the list contains an
 * unserializable template argument such as a template-template.
 */
std::vector<std::string> getTemplateParameterStrings(
    clang::ASTContext &context,
    const clang::ArrayRef<clang::TemplateArgument> &params);

/**
 * Convert a list of template arguments to a single <T0, T1, ...> string.
 *
 * This formats and concatenates the result of getTemplateParameterStrings()
 * for a given function declaration.
 */
std::string getTemplateParameterString(const clang::FunctionDecl *decl);

/**
 * Return whether a return value policy needs to be specfied for a declaration.
 *
 * In certain cases, it is possible to deduce the return value policy that
 * should be used for a given declaration and return type.  This function
 * checks and returns false if the return value policy can be deduced.
 */
bool needsReturnValuePolicy(const clang::NamedDecl *decl,
                            clang::QualType return_type);

/**
 * Returns the minimum and maximum number of arguments that a function can take.
 *
 * This is possible when the function takes some number of default arguments.
 */
std::pair<unsigned, unsigned> getFunctionArgumentRange(
    const clang::FunctionDecl *decl);

} // namespace util
} // namespace chimera

#endif // __CHIMERA_UTIL_H__
