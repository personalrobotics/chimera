#ifndef __CHIMERA_UTIL_H__
#define __CHIMERA_UTIL_H__

#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/CompilerInstance.h>

namespace chimera
{
namespace util
{

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
const clang::NamedDecl* resolveDeclaration(clang::CompilerInstance *ci,
                                           const llvm::StringRef declStr);

/**
 * Resolve a namespace string within the scope of a compiler instance.
 *
 * This function parses the provided namespace string as a single line of the
 * form `namespace [nsStr] {};` within the AST that is currently loaded by the
 * provided compiler instance.
 *
 * If it can be resolved to a namespace declaration, the canonical clang::Decl
 * pointer associated with the namespace will be returned, otherwise NULL.
 */
const clang::NamespaceDecl* resolveNamespace(clang::CompilerInstance *ci,
                                             const llvm::StringRef nsStr);

} // namespace util
} // namespace chimera

#endif // __CHIMERA_UTIL_H__