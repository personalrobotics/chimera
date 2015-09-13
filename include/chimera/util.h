#ifndef __CHIMERA_UTIL_H__
#define __CHIMERA_UTIL_H__

#include <clang/AST/ASTContext.h>
#include <clang/AST/Type.h>
#include <clang/Frontend/CompilerInstance.h>

namespace chimera
{
namespace util
{

clang::QualType findType(clang::CompilerInstance *ci,
                         const llvm::StringRef typeName);

} // namespace util
} // namespace chimera

#endif // __CHIMERA_UTIL_H__