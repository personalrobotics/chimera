#ifndef __CHIMERA_UTIL_H__
#define __CHIMERA_UTIL_H__

#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/CompilerInstance.h>

namespace chimera
{
namespace util
{

const clang::NamedDecl* resolveDeclaration(clang::CompilerInstance *ci,
                                           const llvm::StringRef declStr);

const clang::NamedDecl* resolveNamespace(clang::CompilerInstance *ci,
                                         const llvm::StringRef nsStr);

} // namespace util
} // namespace chimera

#endif // __CHIMERA_UTIL_H__